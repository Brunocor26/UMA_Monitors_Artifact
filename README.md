# Artifact: Universal Microservices for Application Monitoring

Este repositório contém o artifact da tese de licenciatura sobre Microserviços Universais para Monitorização de Aplicações. O projeto demonstra como monitores gerados a partir de fórmulas RMTLD3 podem ser executados em ambientes heterogêneos (Nativo, WASM e Embedded).

---

## Estrutura do Projeto

A organização do projeto separa os **monitores** (o que é monitorado) dos **runners** (onde é monitorado):

*   **`core/`**: Biblioteca `rtmlib` (submódulo git) contendo o motor de avaliação e buffers.
*   **`monitors/`**: Fórmulas RMTLD3 e seus cabeçalhos C++ gerados.
    *   `duration_formula/`: Fórmula `duration of a in 0..15 >= 5`.
    *   `until_formula/`: Fórmula `a U[10s] b`.
*   **`runners/`**: Implementações hospedeiras para cada plataforma.
    *   `native/`: Aplicação C++11 para desktop.
    *   `wasm/`: Microserviço em WebAssembly (WASI).
    *   `pico/`: Implementação para Raspberry Pi Pico (RP2040).
*   **`scripts/`**: Scripts de automação (setup, patches, build).
*   **`docs/`**: Documentação detalhada, guias e diagramas.

---

## Início Rápido

### 1. Configuração Inicial
Após clonar o repositório, execute o script de setup para configurar os submódulos e aplicar os patches necessários:

```bash
./scripts/setup.sh
```

### 2. Compilação — Nativo (Desktop)
```bash
cd runners/native
./compilar.sh
./monitor
```

### 3. Compilação — WebAssembly (WASI)
Requer o [WASI-SDK](https://github.com/WebAssembly/wasi-sdk) em `/opt/wasi-sdk`.
```bash
cd runners/wasm
./run.sh
iwasm build/monitor.wasm
```

### 4. Raspberry Pi Pico
Veja as instruções em `runners/pico/README.md`.

---

## Documentação Detalhada

*   [Guia de Geração de Monitores](docs/guides/generate_monitors.md)
*   [Detalhes do Build WASM](docs/guides/WASM_BUILD.md)
*   [Notas sobre WASI e Compatibilidade](docs/guides/solucao.md)

---
*Este artifact faz parte do projeto de tese na UMA.*
