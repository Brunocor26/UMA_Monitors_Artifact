/**
 * servidor.cpp — Microserviço WASM (WASI) de controlo da ventoinha.
 *
 * Fecha a malha de controlo:
 *
 *   PulseApp ──UDP "RPM=..,Frequency=.."──▶ este servidor
 *        │  faz parse do RPM recebido
 *        │  controlador proporcional: aproxima o RPM do alvo
 *        ▼
 *   ──UDP "DUTY=<0..100>"──▶ FanApp ──ioctl PWM──▶ /dev/pwm0
 *
 * Como WASI puro não tem ioctl, o PWM em si é feito pela app NuttX FanApp;
 * este servidor apenas calcula o duty e envia-o por UDP.
 *
 * Controlo: a cada amostra de RPM ajusta o duty no sentido do erro
 *   (RPM < alvo → sobe duty; RPM > alvo → desce), com um passo proporcional
 *   ao erro e limitado. Dentro da banda de tolerância mantém o duty.
 */
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#ifdef __wasi__
#include <wasi_socket_ext.h>
#endif

/* ───────────────────────── Parâmetros de controlo ───────────────────────── */
#define TARGET_RPM     600    /* RPM desejado para a ventoinha             */
#define RPM_TOLERANCE  50     /* meia-largura da banda "dentro do alvo"    */

#define DUTY_MIN       0
#define DUTY_MAX       100
#define DUTY_INIT      50     /* duty inicial enviado ao FanApp            */

#define RPM_PER_STEP   100    /* RPM de erro por cada 1% de ajuste de duty */
#define MAX_STEP       5      /* limite do ajuste de duty por amostra (%)  */

/* ───────────────────────── Destino (FanApp) ────────────────────────────── */
#define FAN_HOST_DEFAULT "127.0.0.1"
#define FAN_PORT_DEFAULT 5002
#define LISTEN_PORT_DEFAULT 5001

static int clampi(int v, int lo, int hi)
{
    return (v < lo) ? lo : (v > hi) ? hi : v;
}

int main(int argc, char *argv[])
{
    int listen_port = LISTEN_PORT_DEFAULT;
    int fan_port    = FAN_PORT_DEFAULT;
    const char *fan_host = FAN_HOST_DEFAULT;

    int sockfd, sendfd, ret;
    char buffer[1024] = { 0 };
    struct sockaddr_in addr     = { 0 };
    struct sockaddr_in fan_addr = { 0 };

    /* stdout sob WASI é totalmente bufferizado; desliga o buffer para que
     * cada mensagem apareça de imediato. */
    setvbuf(stdout, NULL, _IONBF, 0);

    /* Args: [listen_port] [fan_port] [fan_host] */
    if (argc > 1)
        listen_port = atoi(argv[1]);
    if (argc > 2)
        fan_port = atoi(argv[2]);
    if (argc > 3)
        fan_host = argv[3];

    /* Socket de receção (telemetria do PulseApp) */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Create recv socket failed");
        return EXIT_FAILURE;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(listen_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        return EXIT_FAILURE;
    }

    /* Socket de envio (comando de duty para o FanApp) */
    sendfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sendfd < 0) {
        perror("Create send socket failed");
        close(sockfd);
        return EXIT_FAILURE;
    }

    fan_addr.sin_family = AF_INET;
    fan_addr.sin_port = htons(fan_port);
    if (inet_pton(AF_INET, fan_host, &fan_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid fan host: %s\n", fan_host);
        close(sockfd);
        close(sendfd);
        return EXIT_FAILURE;
    }

    printf("[Fan Controller] Listening on UDP %d, fan at %s:%d (target=%d RPM)\n",
           listen_port, fan_host, fan_port, TARGET_RPM);

    int duty = DUTY_INIT;

    /* Envia o duty inicial para o FanApp arrancar a partir de um valor conhecido. */
    {
        char cmd[32];
        int len = snprintf(cmd, sizeof(cmd), "DUTY=%d\n", duty);
        sendto(sendfd, cmd, len, 0, (struct sockaddr *)&fan_addr, sizeof(fan_addr));
    }

    while (1) {
        ret = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, NULL, NULL);
        if (ret < 0) {
            perror("recvfrom failed");
            break;
        }
        buffer[ret] = '\0';

        /* Espera "RPM=<n>,Frequency=<n>" (formato enviado pelo PulseApp). */
        unsigned long rpm = 0, freq = 0;
        if (sscanf(buffer, "RPM=%lu,Frequency=%lu", &rpm, &freq) < 1) {
            printf("[Fan Controller] Ignored: %s", buffer);
            continue;
        }

        /* Erro: positivo quando o RPM está abaixo do alvo. */
        long err = (long)TARGET_RPM - (long)rpm;

        if (labs(err) > RPM_TOLERANCE) {
            /* Passo proporcional ao erro, limitado a +/-MAX_STEP. */
            int step = (int)(err / RPM_PER_STEP);
            if (step == 0)
                step = (err > 0) ? 1 : -1;
            step = clampi(step, -MAX_STEP, MAX_STEP);

            duty = clampi(duty + step, DUTY_MIN, DUTY_MAX);

            char cmd[32];
            int len = snprintf(cmd, sizeof(cmd), "DUTY=%d\n", duty);
            if (sendto(sendfd, cmd, len, 0, (struct sockaddr *)&fan_addr,
                       sizeof(fan_addr)) < 0) {
                perror("sendto fan failed");
            }
        }

        printf("[Fan Controller] RPM=%lu target=%d err=%ld | duty=%d%%\n",
               rpm, TARGET_RPM, err, duty);
    }

    close(sendfd);
    close(sockfd);
    return EXIT_SUCCESS;
}
