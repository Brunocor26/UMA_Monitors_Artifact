# Artifact: Universal Microservices for Application Monitoring

Este repositório contém o artifact do projeto de licenciatura sobre Microserviços Universais para Monitorização de Aplicações. O projeto demonstra como monitores gerados a partir de fórmulas RMTLD3 podem ser executados em ambientes heterogêneos (Nativo, WASM e Embedded) com um mesmo binário WASM, através do [WAMR](https://github.com/bytecodealliance/wasm-micro-runtime) (WebAssembly Micro Runtime).

---

## Estrutura do Projeto

A organização do projeto separa os **monitores** (o que é monitorado) dos **runners** (onde é monitorado):

*   **`rtmlib/`**: Biblioteca `rtmlib` (submódulo git) que contém o motor de avaliação e buffers.
*   **`monitors/`**: Fórmulas RMTLD3 e seus cabeçalhos C++ gerados.
    *   `duration_monitor/`: Fórmula `duration of a in 0..15 >= 5`.
    *   `until_monitor/`: Fórmula `a U[10s] b`.
    *   `final_monitor/`: Fórmula `a -> (a U[<10s] b)` usada no *running example* (supervisão da ventoinha).
*   **`runners/`**: Implementações hospedeiras para cada plataforma.
    *   `native/`: Aplicação C++11 para desktop.
    *   `wasm/`: Microserviço em WebAssembly (WASI).
*   **`intoNuttx/`**: *Running example* no NuttX — malha fechada de controlo e supervisão de uma ventoinha, em que cada peça é um microserviço que comunica por UDP:

    ```
    pulse_reader --RPM--> wasm_server --DUTY--> fan_app --PWM--> ventoinha
                              |  (monitor RMTLD3)
                              +-----------------ALARM--> led_app --GPIO--> LED
    ```

    *   `pulse_reader/`: app NuttX que conta pulsos GPIO e calcula RPM/frequência (sensor).
    *   `wasm_server/`: microserviço WASM (WASI) que controla o duty para um RPM-alvo e supervisiona com um monitor RMTLD3 (cérebro).
    *   `fan_app/`: app NuttX que aplica o PWM na ventoinha via ioctl (atuador).
    *   `led_app/`: app NuttX que pisca um LED quando o monitor deteta uma violação (alarme).
    *   `init.sh`: arranca as quatro peças de uma vez no NSH.
*   **`scripts/`**: Scripts de automação (setup, patches, build).
*   **`docs/`**: Documentação detalhada, guias e diagramas.

---

## Início Rápido

### 1. Configuração Inicial
Após clonar o repositório, execute o script de setup para configurar os submódulos e aplicar os patches necessários:

```bash
./scripts/setup.sh
```

### 2. Compilação - Nativo (Desktop)
```bash
cd runners/native
./compilar.sh
./monitor
```

### 3. Compilação - WebAssembly (WASI)
Requer o [WASI-SDK](https://github.com/WebAssembly/wasi-sdk) em `/opt/wasi-sdk`.
```bash
cd runners/wasm
./run.sh
iwasm build/monitor.wasm
```

---

## Documentação Detalhada

*   [Guia de Geração de Monitores](docs/guides/generate_monitors.md)
*   [Detalhes do Build WASM](docs/guides/WASM_BUILD.md)
*   [Notas sobre WASI e Compatibilidade](docs/guides/solucao.md)
