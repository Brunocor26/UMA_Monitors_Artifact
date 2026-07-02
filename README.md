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

## Benchmark (Avaliação de Desempenho)

O script `benchmark.sh` permite comparar o desempenho e o tamanho em disco (footprint) do microserviço de monitorização entre duas abordagens de isolamento e execução:
1. **WebAssembly (WASM):** O módulo compilado (`servidor.wasm`) executado diretamente através do runtime WAMR (`iwasm`).
2. **Contentorização (Docker):** O executável nativo compilado estaticamente para 32-bits e empacotado num contentor leve baseado em Alpine Linux i386.

### Métricas Avaliadas

*   **Tamanho em Disco (Footprint):** Compara o tamanho do ficheiro `.wasm` gerado com o tamanho total da imagem Docker (`uma-servidor-i386`).
*   **Tempo de Arranque e Resposta:** Mede (usando a ferramenta `hyperfine`) o tempo necessário para inicializar a instância (contentor vs runtime WASM), processar uma mensagem de teste enviada via UDP (simulando a leitura de RPM da ventoinha) e terminar o processo.

### Pré-requisitos

Para executar o benchmark, garanta que tem instalado no seu sistema:
*   [Docker](https://www.docker.com/)
*   [hyperfine](https://github.com/sharkdp/hyperfine) (ferramenta de benchmarking em linha de comandos)
*   [WAMR (iwasm)](https://github.com/bytecodealliance/wasm-micro-runtime) no seu PATH
*   Python 3 (utilizado pelo script para enviar pacotes UDP de teste)

### Como Executar

Execute o seguinte comando a partir da raiz do repositório:

```bash
./benchmark.sh
```

O script irá:
1. Construir a imagem Docker local (`uma-servidor-i386`).
2. Imprimir o tamanho da imagem Docker e do ficheiro `.wasm`.
3. Executar o benchmark com o `hyperfine` para a versão WASM (20 execuções com 3 de aquecimento).
4. Executar o benchmark com o `hyperfine` para a versão Docker (com 3 de aquecimento).

---

## Documentação Detalhada

*   [Guia de Geração de Monitores](docs/guides/generate_monitors.md)
*   [Detalhes do Build WASM](docs/guides/WASM_BUILD.md)
*   [Notas sobre WASI e Compatibilidade](docs/guides/solucao.md)
