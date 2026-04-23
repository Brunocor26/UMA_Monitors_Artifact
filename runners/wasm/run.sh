#!/bin/bash
# Script de compilação WASM (WASI) para o monitor RMTLD3.
# Requer o WASI-SDK instalado em /opt/wasi-sdk.

set -e

WASI_SDK_PATH="/opt/wasi-sdk"
CLANG_PP="${WASI_SDK_PATH}/bin/clang++"

if [ ! -f "$CLANG_PP" ]; then
    echo "Erro: WASI-SDK não encontrado em $WASI_SDK_PATH"
    exit 1
fi

mkdir -p build

echo "--- A compilar para para WASM (WASI) ---"
$CLANG_PP -O3 \
    --target=wasm32-wasi \
    --sysroot=${WASI_SDK_PATH}/share/wasi-sysroot \
    -Wall -Wextra -std=gnu++11 -DRTMLIB_ENABLE_MAP_SORT \
    -D__i386__ -D__x86__ \
    -Duseconds_t=uint32_t \
    -fno-exceptions \
    -include wasi_compat.h \
    -I../../core/src \
    -I../../monitors/until_formula/headers \
    main.cpp wasi_stubs.cpp -o build/monitor.wasm

echo "--- Sucesso! Binário gerado em: build/monitor.wasm ---"
echo "Para rodar com iwasm: iwasm build/monitor.wasm"
