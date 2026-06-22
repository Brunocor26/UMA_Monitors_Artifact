# fan_app

Aplicacao NuttX built-in que atua o PWM da ventoinha. Recebe por UDP comandos
`DUTY=<0..100>` (enviados pelo `servidor.wasm`) e aplica-os num canal PWM via
ioctl. E a ponta de atuacao da malha de controlo:

```
PulseApp --RPM--> servidor.wasm --DUTY--> fan_app --PWM--> ventoinha
```

## Como funciona

A cada datagrama `DUTY=<n>`, converte a percentagem (0..100) para ponto fixo
16.16 (`ub16_t`, onde `0x10000` = 100%, limitado a `0xffff`) e aplica no canal
via `PWMIOC_SETCHARACTERISTICS` + `PWMIOC_START` sobre o device PWM.

## Uso

```
fan_app [-p <port>] [-f <freq_hz>] [<pwm-device>]
```

| Argumento | Default | Descricao |
|---|---|---|
| `-p <port>` | `5002` | Porta UDP de escuta |
| `-f <freq_hz>` | `25000` | Frequencia do PWM (Hz) |
| `<pwm-device>` | `/dev/pwm0` | Caminho do device PWM |

## Integracao no NuttX

Copiar a pasta para `apps/CustomApps/fan_app` (ou `FanApp`) e registar como
built-in (os `Makefile`/`Kconfig`/`Make.defs` incluidos seguem o processo
padrao). Ativar `CONFIG_CUSTOM_APPS_FAN=y` e ter `CONFIG_PWM=y` com o device
PWM registado (ex.: `/dev/pwm3` no RP2350).

Requer o ioctl `PWMIOC_SETCHARACTERISTICS`/`PWMIOC_START` de
`<nuttx/timers/pwm.h>`.
