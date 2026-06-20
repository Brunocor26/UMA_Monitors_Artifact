# Servidor WASM — Controlador da ventoinha

Microserviço WebAssembly (WASI), executado sob `iwasm` no NuttX, que **fecha a
malha de controlo** da ventoinha:

```
PulseApp ──UDP "RPM=..,Frequency=.."──▶ servidor WASM (porta 5001)
                                            │  parse do RPM
                                            │  controlador proporcional
                                            ▼
                                       UDP "DUTY=<0..100>" ──▶ FanApp (porta 5002)
                                                                  │ ioctl PWM
                                                                  ▼ /dev/pwm0
```

Como o WASI puro não tem `ioctl`, o servidor WASM **não** mexe no PWM diretamente:
calcula o *duty* e envia-o por UDP à app NuttX `FanApp`
(`apps/CustomApps/FanApp/`), que faz o `ioctl` no device PWM.

## Controlo

A cada amostra de RPM recebida, o servidor compara com o alvo e ajusta o duty no
sentido do erro:

- `err = TARGET_RPM − RPM`.
- Dentro da banda de tolerância (`|err| <= RPM_TOLERANCE`) → mantém o duty.
- Fora da banda → ajusta o duty por um passo **proporcional ao erro**, limitado a
  `±MAX_STEP` (`RPM < alvo` → sobe; `RPM > alvo` → desce). Clamp 0..100.

Parâmetros (no topo de `servidor.cpp`):

| `#define`       | Significado                                   |
|-----------------|-----------------------------------------------|
| `TARGET_RPM`    | RPM alvo                                       |
| `RPM_TOLERANCE` | meia-largura da banda "dentro do alvo"         |
| `RPM_PER_STEP`  | RPM de erro por cada 1% de ajuste de duty      |
| `MAX_STEP`      | ajuste máximo de duty por amostra (%)          |
| `DUTY_INIT`     | duty inicial enviado ao FanApp                 |

## Compilação

Requer o WASI-SDK em `/opt/wasi-sdk`.

> **AVISO:** ajustar `WAMR_ROOT` no `build.sh` para o caminho do
> wasm-micro-runtime nesta máquina.

```bash
./build.sh          # gera build/servidor.wasm
```

## Execução

```bash
# Argumentos: [listen_port] [fan_port] [fan_host]
iwasm build/servidor.wasm           # defaults: 5001, 5002, 127.0.0.1
```

No NuttX, ver `intoNuttx/init.sh` para arrancar `fan_app`, `servidor.wasm` e
`pulse_app` de uma só vez. Em alvos com pouca RAM, passar
`--heap-size`/`--stack-size` ao `iwasm` para reduzir o consumo.
