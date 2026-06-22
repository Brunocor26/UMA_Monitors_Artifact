# Servidor WASM - Controlo + supervisao da ventoinha

Microservico WebAssembly (WASI), executado sob `iwasm` no NuttX, que fecha a
malha de controlo da ventoinha e supervisiona o seu comportamento com um
monitor RMTLD3:

```
PulseApp --UDP "RPM=..,Frequency=.."--> servidor WASM (porta 5001)
                                            |  parse do RPM
                                            |  [CONTROLO]   controlador proporcional
                                            |  [SUPERVISAO] monitor RMTLD3
                                            v
                          --"DUTY=<0..100>"--> FanApp (5002) --ioctl PWM--> /dev/pwm0
                          --"ALARM=<0|1>"----> LedApp (5003) --GPIO------> LED
```

Como o WASI puro nao tem `ioctl`, o servidor nao mexe no hardware: calcula o
duty e o estado de alarme e envia-os por UDP as apps NuttX `FanApp` e `LedApp`,
que fazem o acesso aos perifericos.

## Controlo

A cada amostra de RPM, compara com `TARGET_RPM` e ajusta o duty no sentido do
erro, por um passo proporcional limitado a `+/-MAX_STEP`. Dentro da banda de
tolerancia (`RPM_TOLERANCE`) mantem o duty.

## Supervisao (monitor RMTLD3)

Em paralelo ao controlo, o monitor verifica a formula `a -> (a U[<10s] b)`
(`a` = RPM fora do alvo, `b` = RPM no alvo): "se sai do alvo, tem de recuperar
em menos de 10s". Quando o veredito e FALSE (nao recuperou a tempo), o servidor:

- imprime um alarme; e
- envia `ALARM=1` ao LedApp (que pisca o LED). Ao recuperar (veredito TRUE)
  envia `ALARM=0`.

O monitor e integrado de forma generica (`mon_template.cpp` + `monitor_api.h`):
o `build.sh` descobre o hash dos cabecalhos gerados pelo `rmtld3synth` em
`monitors/final_monitor/headers/`, por isso regenerar a formula so exige
recompilar. Como as formulas RMTLD3 sao de futuro limitado, o veredito e
avaliado num instante atrasado (`t - janela - margem`) e regista-se um evento
por segundo.

## Parametros (topo de `servidor.cpp`)

| `#define`                        | Significado                                   |
|----------------------------------|-----------------------------------------------|
| `TARGET_RPM`                     | RPM alvo                                       |
| `RPM_TOLERANCE`                  | meia-largura da banda "dentro do alvo"         |
| `RPM_PER_STEP` / `MAX_STEP`      | ganho e passo maximo de duty por amostra       |
| `MON_WINDOW`                     | janela da formula (10s)                        |
| `MONITOR_FORCE_MAX_ON_VIOLATION` | 1 = forca duty maximo na violacao; 0 = so alarme |

## Compilacao

Requer o WASI-SDK em `/opt/wasi-sdk`.

> AVISO: ajustar `WAMR_ROOT` no `build.sh` para o caminho do wasm-micro-runtime
> nesta maquina.

```bash
./build.sh          # gera build/servidor.wasm
```

## Execucao

```bash
# Args: [listen_port] [fan_port] [fan_host] [led_port]
iwasm build/servidor.wasm 5001 5002 127.0.0.1 5003
```

O servidor usa um unico socket UDP (recebe e envia), para nao esgotar o numero
de conexoes UDP do NuttX (`CONFIG_NET_UDP_MAX_CONNS`). Ver `intoNuttx/init.sh`
para arrancar `fan_app`, `led_app`, `servidor.wasm` e `pulse_app` de uma vez.
Em alvos com pouca RAM, passar `--stack-size` ao `iwasm`.
