/****************************************************************************
 * apps/CustomApps/LedApp/LedApp.c
 *
 * Indicador de alarme por LED.
 *
 * Recebe por UDP comandos "ALARM=<0|1>" enviados pelo microserviço WASM
 * (servidor.wasm). Enquanto o alarme está ativo (1), faz piscar um LED ligado
 * a um pino GPIO de saída; quando inativo (0), apaga o LED.
 *
 *   servidor WASM --"ALARM=1"--> LedApp --GPIO--> LED a piscar
 *
 * É a ponta de atuação visual da supervisão: o monitor RMTLD3 deteta que a
 * ventoinha não recuperou o alvo em <10s (veredito FALSE) e o servidor pede
 * o alarme. Como o WASI (WASM) não tem ioctl, é esta app nativa que mexe no
 * GPIO.
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <nuttx/ioexpander/gpio.h>

#define DEFAULT_PORT      5003
#define DEFAULT_DEVPATH   "/dev/gpio21"
#define BLINK_MS          200      /* meio-período do pisca (ms)            */

/* Ligação do LED:
 *   1 = ativo-alto  (GP21 -> R -> anodo -> catodo -> GND;   acende com 1)
 *   0 = ativo-baixo (3V3  -> R -> anodo -> catodo -> GP21;  acende com 0)
 */
#define LED_ACTIVE_LOW    1

static void show_usage(const char *progname)
{
  fprintf(stderr, "USAGE: %s [-p <port>] [<gpio-device>]\n", progname);
  fprintf(stderr, "Where:\n");
  fprintf(stderr, "\t<gpio-device>: GPIO de saída do LED (default: %s)\n",
          DEFAULT_DEVPATH);
  fprintf(stderr, "\t-p <port>:     porta UDP de escuta (default: %d)\n",
          DEFAULT_PORT);
}

static void led_write(int fd, bool on)
{
#if LED_ACTIVE_LOW
  ioctl(fd, GPIOC_WRITE, (unsigned long)(on ? 0 : 1));
#else
  ioctl(fd, GPIOC_WRITE, (unsigned long)(on ? 1 : 0));
#endif
}

int led_app_main(int argc, char *argv[])
{
  const char *devpath = DEFAULT_DEVPATH;
  int port = DEFAULT_PORT;
  int opt;
  int fd;
  int sockfd;
  struct sockaddr_in addr;
  struct timeval tv;
  char buffer[64];
  bool alarm_active = false;
  bool led_on = false;
  int ret;

  optind = 0;
  while ((opt = getopt(argc, argv, "p:")) != -1)
    {
      switch (opt)
        {
          case 'p':
            port = atoi(optarg);
            break;
          default:
            show_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

  if (optind < argc)
    {
      devpath = argv[optind];
    }

  fd = open(devpath, O_RDWR);
  if (fd < 0)
    {
      fprintf(stderr, "ERROR: Failed to open %s\n", devpath);
      return EXIT_FAILURE;
    }

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
    {
      fprintf(stderr, "ERROR: Failed to create socket\n");
      close(fd);
      return EXIT_FAILURE;
    }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family      = AF_INET;
  addr.sin_port        = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
      fprintf(stderr, "ERROR: Bind failed\n");
      close(sockfd);
      close(fd);
      return EXIT_FAILURE;
    }

  /* Timeout na receção: assim o recvfrom regressa periodicamente para
   * alternar o estado do LED enquanto não chegam mensagens. */

  tv.tv_sec  = 0;
  tv.tv_usec = BLINK_MS * 1000;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

  led_write(fd, false);

  printf("Led Alarm: device=%s, listening on UDP %d...\n", devpath, port);

  while (1)
    {
      ret = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, NULL, NULL);

      if (ret > 0)
        {
          int v = 0;
          buffer[ret] = '\0';
          if (sscanf(buffer, "ALARM=%d", &v) == 1)
            {
              alarm_active = (v != 0);
              if (!alarm_active)
                {
                  led_on = false;
                  led_write(fd, false);     /* alarme limpo -> LED apagado */
                }
            }
          continue;
        }

      /* ret <= 0: timeout (EAGAIN/EWOULDBLOCK) ou erro. */

      if (ret < 0 && errno != EAGAIN && errno != EWOULDBLOCK
          && errno != EINTR)
        {
          fprintf(stderr, "ERROR: recvfrom failed (%d)\n", errno);
          break;
        }

      if (alarm_active)
        {
          led_on = !led_on;             /* pisca enquanto em alarme */
          led_write(fd, led_on);
        }
    }

  led_write(fd, false);
  close(sockfd);
  close(fd);
  return EXIT_SUCCESS;
}
