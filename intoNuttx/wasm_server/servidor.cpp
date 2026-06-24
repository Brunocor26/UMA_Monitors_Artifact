/**
 * servidor.cpp - Microserviço WASM (WASI) de controlo + supervisão da ventoinha.
 *
 *   PulseApp --UDP "RPM=..,Frequency=.."--> este servidor
 *        |  parse do RPM
 *        |  [CONTROLO]  controlador proporcional -> novo duty
 *        |  [SUPERVISÃO] monitor RMTLD3 verifica "a -> (a U[<10s] b)"
 *        v
 *   --UDP "DUTY=<0..100>"--> FanApp --ioctl PWM--> /dev/pwm0
 *
 * O controlo e a supervisão são independentes:
 *   - O controlador proporcional persegue TARGET_RPM (mexe no duty).
 *   - O monitor apenas observa: dá veredito FALSE quando o RPM saiu do alvo
 *     e não recuperou em <10s. Na violação, dispara uma ação de supervisão
 *     (alarme; opcionalmente força duty máximo).
 *
 * 'a' = RPM fora do alvo, 'b' = RPM no alvo (proposições do monitor).
 */
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#ifdef __wasi__
#include <wasi_socket_ext.h>
#endif

#include "monitor_api.h"

/* ------------------------- Parâmetros de controlo ------------------------- */
#define TARGET_RPM_DEFAULT     600    /* RPM desejado default                      */
#define RPM_TOLERANCE  50     /* meia-largura da banda "dentro do alvo"    */

#define DUTY_MIN       0
#define DUTY_MAX       100
#define DUTY_INIT      50
#define RPM_PER_STEP   100    /* ganho integral: Ki = 1 / RPM_PER_STEP     */

/* ------------------------- Parâmetros de supervisão ----------------------- */
#define MON_WINDOW     10     /* janela da fórmula (a U[<10s] b)           */
#define EVAL_MARGIN    2      /* margem p/ o trace cobrir a janela futura  */
/* Ação na violação: 1 = força duty máximo (fallback de segurança);
 *                   0 = só alarme (o monitor não interfere no controlo).  */
#define MONITOR_FORCE_MAX_ON_VIOLATION 0

/* ------------------------- Destinos (FanApp / LedApp) -------------------- */
#define FAN_HOST_DEFAULT "127.0.0.1"
#define FAN_PORT_DEFAULT 5002
#define LED_HOST_DEFAULT "127.0.0.1"
#define LED_PORT_DEFAULT 5003
#define LISTEN_PORT_DEFAULT 5001

static int value_limit_between(int v, int lo, int hi)
{
    return (v < lo) ? lo : (v > hi) ? hi : v;
}

static const char *verdict_str(int v)
{
    return (v == MON_TRUE) ? "TRUE" : (v == MON_FALSE) ? "FALSE" : "UNKNOWN";
}

static double now_seconds(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static int send_duty(int sockfd, const struct sockaddr_in *fan, int duty)
{
    char cmd[32];
    int len = snprintf(cmd, sizeof(cmd), "DUTY=%d\n", duty);
    return sendto(sockfd, cmd, len, 0, (const struct sockaddr *)fan, sizeof(*fan));
}

int main(int argc, char *argv[])
{
    int listen_port = LISTEN_PORT_DEFAULT;
    int fan_port    = FAN_PORT_DEFAULT;
    int led_port    = LED_PORT_DEFAULT;
    const char *fan_host = FAN_HOST_DEFAULT;

    int sockfd, ret;
    char buffer[1024] = { 0 };
    struct sockaddr_in addr     = { 0 };
    struct sockaddr_in fan_addr = { 0 };
    struct sockaddr_in led_addr = { 0 };

    setvbuf(stdout, NULL, _IONBF, 0);

    /* Args: [target_rpm] [listen_port] [fan_port] [fan_host] [led_port] */
    int target_rpm = TARGET_RPM_DEFAULT;
    if (argc > 1) target_rpm  = atoi(argv[1]);
    if (argc > 2) listen_port = atoi(argv[2]);
    if (argc > 3) fan_port    = atoi(argv[3]);
    if (argc > 4) fan_host    = argv[4];
    if (argc > 5) led_port    = atoi(argv[5]);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { perror("recv socket"); return EXIT_FAILURE; }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(listen_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(sockfd); return EXIT_FAILURE;
    }

    /* Um unico socket UDP serve para receber (bind em listen_port) e enviar
     * (sendto para fan/led). Assim usamos so 1 conexao UDP do NuttX. */

    fan_addr.sin_family = AF_INET;
    fan_addr.sin_port = htons(fan_port);
    if (inet_pton(AF_INET, fan_host, &fan_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid fan host: %s\n", fan_host);
        close(sockfd); return EXIT_FAILURE;
    }

    /* O LedApp está no mesmo host que o FanApp (loopback do dispositivo). */
    led_addr.sin_family = AF_INET;
    led_addr.sin_port = htons(led_port);
    led_addr.sin_addr = fan_addr.sin_addr;

    printf("[Fan Controller] Listening on UDP %d, fan at %s:%d, led at %s:%d "
           "(target=%d RPM)\n",
           listen_port, fan_host, fan_port, fan_host, led_port, target_rpm);

    int duty = DUTY_INIT;
    double duty_internal = DUTY_INIT;
    int alarm = -1;            /* -1 desconhecido, 0 limpo, 1 em alarme */
    double t_start = now_seconds();
    long last_ts = -1;

    send_duty(sockfd, &fan_addr, duty);

    while (1) {
        ret = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, NULL, NULL);
        if (ret < 0) { perror("recvfrom"); break; }
        buffer[ret] = '\0';

        unsigned long rpm = 0, freq = 0;
        if (sscanf(buffer, "RPM=%lu,Frequency=%lu", &rpm, &freq) < 1) {
            printf("[Fan Controller] Ignored: %s", buffer);
            continue;
        }

        double t = now_seconds() - t_start;
        long its = (long)t;
        
        // 1. Calcula o Erro: Diferença entre o target_rpm e o RPM atual
        long err = (long)target_rpm - (long)rpm;
        bool out_of_target = (labs(err) > RPM_TOLERANCE) ? 1 : 0;

        /* CONTROLO: controlador integral discreto (forma incremental)
         *   u_k = sat( u_{k-1} + Ki * err ),  Ki = 1 / RPM_PER_STEP */
        // 2 e 3. Calcula a Correção e Ajusta o Duty Cycle: 
        // Acumula a correção em precisão dupla para não perder valores fracionários
        duty_internal += (double)err / RPM_PER_STEP;
        duty_internal = (duty_internal < DUTY_MIN) ? DUTY_MIN : (duty_internal > DUTY_MAX) ? DUTY_MAX : duty_internal;
        duty = (int)duty_internal;
        
        // 4. Atua na Ventoinha: Envia o novo valor ajustado para a ventoinha via UDP
        if (send_duty(sockfd, &fan_addr, duty) < 0) perror("sendto fan");

        /* -- SUPERVISÃO: monitor RMTLD3 (um evento por segundo) -- */
        // 5. Monitorização: Alimenta os dados para o monitor a cada 1 segundo
        if (its > last_ts) {
            mon_push(out_of_target, (double)its);
            last_ts = its;
        }
        /* Avalia num instante atrasado, onde o trace já cobre a janela. */
        int vd = (its >= MON_WINDOW + EVAL_MARGIN)
                     ? mon_eval((double)(its - MON_WINDOW - EVAL_MARGIN))
                     : MON_UNKNOWN;

        if (vd == MON_FALSE) {
            printf("[MONITOR] VIOLACAO: RPM fora do alvo sem recuperar em <%ds\n",
                   MON_WINDOW);
#if MONITOR_FORCE_MAX_ON_VIOLATION
            duty = DUTY_MAX;
            send_duty(sockfd, &fan_addr, duty);
#endif
        }

        /* Avisa o LedApp na transição do veredito: FALSE -> piscar (ALARM=1),
         * TRUE -> apagar (ALARM=0). UNKNOWN mantém o estado anterior. */
        int new_alarm = (vd == MON_FALSE) ? 1 : (vd == MON_TRUE) ? 0 : alarm;
        if (new_alarm != alarm && new_alarm >= 0) {
            alarm = new_alarm;
            char cmd[16];
            int len = snprintf(cmd, sizeof(cmd), "ALARM=%d\n", alarm);
            sendto(sockfd, cmd, len, 0, (struct sockaddr *)&led_addr,
                   sizeof(led_addr));
        }

        printf("[Fan Controller] RPM=%lu target=%d err=%ld | mon=%s | duty=%d%%\n",
               rpm, target_rpm, err, verdict_str(vd), duty);
    }

    close(sockfd);
    return EXIT_SUCCESS;
}
