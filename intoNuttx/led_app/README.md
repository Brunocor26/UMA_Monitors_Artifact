# led_app

Aplicacao NuttX built-in que sinaliza alarmes num LED. Recebe por UDP comandos
`ALARM=<0|1>` (enviados pelo `servidor.wasm` quando o monitor RMTLD3 da veredito
FALSE) e, enquanto o alarme esta ativo, faz piscar um LED num pino GPIO.

```
servidor.wasm --ALARM=1--> led_app --GPIO--> LED a piscar
```

## Como funciona

Usa `recvfrom` com timeout (`SO_RCVTIMEO`), por isso regressa periodicamente
para alternar o LED enquanto nao chegam mensagens:

- `ALARM=1` -> alarme ativo: alterna o LED a cada `BLINK_MS` (200 ms ~ 2.5 Hz).
- `ALARM=0` -> apaga o LED.

A polaridade da ligacao escolhe-se com `LED_ACTIVE_LOW` (topo do `LedApp.c`):

- `1` = ativo-baixo (`3V3 -> R -> anodo -> catodo -> GPIO`; acende com 0)
- `0` = ativo-alto  (`GPIO -> R -> anodo -> catodo -> GND`; acende com 1)

## Uso

```
led_app [-p <port>] [<gpio-device>]
```

| Argumento | Default | Descricao |
|---|---|---|
| `-p <port>` | `5003` | Porta UDP de escuta |
| `<gpio-device>` | `/dev/gpio21` | GPIO de saida do LED |

## Integracao no NuttX

Copiar a pasta para `apps/CustomApps/led_app` (ou `LedApp`) e registar como
built-in. Ativar `CONFIG_CUSTOM_APPS_LED=y` e `CONFIG_DEV_GPIO=y`.

O pino do LED tem de estar registado como **saida** (`GPIO_OUTPUT_PIN`). No
Raspberry Pi Pico 2 isso faz-se no board, em
`boards/arm/rp23xx/raspberrypi-pico-2/src/rp23xx_gpio.c` (acrescentar o pino ao
array `g_gpiooutputs[]`) e em `include/board.h` (`BOARD_NGPIOOUT`). O device
fica `/dev/gpio<pino>` (ex.: GP21 -> `/dev/gpio21`).

Requer o ioctl `GPIOC_WRITE` de `<nuttx/ioexpander/gpio.h>`.
