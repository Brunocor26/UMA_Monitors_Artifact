#!/bin/bash
# Script de compilação WASM (WASI) para o monitor RMTLD3.
# Requer o WASI-SDK instalado em /opt/wasi-sdk.
# Flags de compilação alternativas https://anoopelias.github.io/posts/wasm-micro-runtime-with-rust/
# Por enquanto nao funciona!

set -e

WASI_SDK_PATH="/opt/wasi-sdk"
CLANG_PP="${WASI_SDK_PATH}/bin/clang++"

if [ ! -f "$CLANG_PP" ]; then
    echo "Erro: WASI-SDK não encontrado em $WASI_SDK_PATH"
    exit 1
fi

mkdir -p build

echo "--- A compilar para WASM ---"
$CLANG_PP -O3 \
        -z stack-size=4096 -Wl,--initial-memory=65536 \
        --sysroot=${WASI_SDK_PATH}/share/wasi-sysroot \
        -include wasi_compat.h \
        -Wl,--export=main -Wl,--export=__main_argc_argv \
        -Wl,--export=__data_end -Wl,--export=__heap_base \
        -Wl,--strip-all,--no-entry \
        -Wl,--allow-undefined \
        -nostdlib \
        -I../../core/src \
        -I../../monitors/until_formula/headers \
        main.cpp wasi_stubs.cpp -o build/monitor_alternative.wasm

echo "--- Sucesso! Binário gerado em: build/monitor.wasm ---"
echo "Para rodar com iwasm: iwasm build/monitor.wasm"
