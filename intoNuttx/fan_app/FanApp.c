/****************************************************************************
 * apps/CustomApps/FanApp/FanApp.c
 *
 * Controlador de PWM da ventoinha.
 *
 * Recebe por UDP comandos "DUTY=<0..100>" enviados pelo microserviço WASM
 * (servidor.wasm) e aplica-os no canal PWM indicado, via ioctl.  É a ponta
 * de atuação da malha:
 *
 *   PulseApp --RPM--> servidor WASM --DUTY--> FanApp --PWM--> ventoinha
 *
 * Como o WASI (WASM) não tem ioctl, é esta app NuttX nativa que faz o
 * acesso ao device PWM.
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <nuttx/timers/pwm.h>

#define DEFAULT_PORT      5002
#define DEFAULT_FREQUENCY 25000      /* 25 kHz */
#define DEFAULT_DEVPATH   "/dev/pwm0"

static void show_usage(const char *progname)
{
  fprintf(stderr,
          "USAGE: %s [-p <port>] [-f <freq_hz>] [<pwm-device>]\n", progname);
  fprintf(stderr, "Where:\n");
  fprintf(stderr, "\t<pwm-device>: PWM device path (default: %s)\n",
          DEFAULT_DEVPATH);
  fprintf(stderr, "\t-p <port>:    UDP listen port (default: %d)\n",
          DEFAULT_PORT);
  fprintf(stderr, "\t-f <freq_hz>: PWM frequency in Hz (default: %d)\n",
          DEFAULT_FREQUENCY);
}

/* Converte percentagem (0..100) para duty em ponto fixo 16.16 (ub16_t),
 * onde 0x10000 representa 100%.  Limita a 0xffff (full-scale válido).
 */
static ub16_t duty_from_percent(int percent)
{
  uint32_t b16;

  if (percent < 0)
    percent = 0;
  if (percent > 100)
    percent = 100;

  b16 = ((uint32_t)percent * 65536u) / 100u;
  if (b16 > 0xffff)
    b16 = 0xffff;

  return (ub16_t)b16;
}

int fan_app_main(int argc, char *argv[])
{
  const char *devpath = DEFAULT_DEVPATH;
  int port = DEFAULT_PORT;
  uint32_t frequency = DEFAULT_FREQUENCY;
  int opt;
  int fd;
  int sockfd;
  struct sockaddr_in addr;
  struct pwm_info_s info;
  char buffer[128];
  int ret;

  optind = 0;
  while ((opt = getopt(argc, argv, "p:f:")) != -1)
    {
      switch (opt)
        {
          case 'p':
            port = atoi(optarg);
            break;
          case 'f':
            frequency = (uint32_t)atoi(optarg);
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

  /* Abre o device PWM. */

  fd = open(devpath, O_RDONLY);
  if (fd < 0)
    {
      fprintf(stderr, "ERROR: Failed to open %s\n", devpath);
      return EXIT_FAILURE;
    }

  /* Socket UDP de receção dos comandos de duty. */

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

  printf("Fan PWM: device=%s freq=%lu Hz, listening on UDP %d...\n",
         devpath, (unsigned long)frequency, port);

  while (1)
    {
      ret = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, NULL, NULL);
      if (ret < 0)
        {
          fprintf(stderr, "ERROR: recvfrom failed\n");
          break;
        }

      buffer[ret] = '\0';

      int percent = 0;
      if (sscanf(buffer, "DUTY=%d", &percent) != 1)
        {
          printf("Fan PWM: ignored '%s'\n", buffer);
          continue;
        }

      /* Prepara as características do PWM e aplica. */

      memset(&info, 0, sizeof(info));
      info.frequency           = frequency;
      info.channels[0].channel = 1;
      info.channels[0].duty    = duty_from_percent(percent);

      if (ioctl(fd, PWMIOC_SETCHARACTERISTICS,
                (unsigned long)((uintptr_t)&info)) < 0)
        {
          fprintf(stderr, "ERROR: PWMIOC_SETCHARACTERISTICS failed\n");
          continue;
        }

      if (ioctl(fd, PWMIOC_START, 0) < 0)
        {
          fprintf(stderr, "ERROR: PWMIOC_START failed\n");
          continue;
        }

      printf("Fan PWM: duty=%d%% (b16=0x%04x)\n",
             percent, (unsigned int)info.channels[0].duty);
    }

  ioctl(fd, PWMIOC_STOP, 0);
  close(sockfd);
  close(fd);
  return EXIT_SUCCESS;
}
