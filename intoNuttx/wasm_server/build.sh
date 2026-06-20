#!/bin/bash
set -e

WASI_SDK_PATH="/opt/wasi-sdk"
CLANG="${WASI_SDK_PATH}/bin/clang"
CLANGPP="${WASI_SDK_PATH}/bin/clang++"

WAMR_ROOT="/home/bruno/Desktop/Projeto/webassembly/wasm-micro-runtime"
HEADER_PATH=$(find "$WAMR_ROOT" -name "wasi_socket_ext.h" | head -n 1)
WRAPPER_SRC=$(find "$WAMR_ROOT" -name "wasi_socket_ext.c" | head -n 1)

WAMR_INC=$(dirname "$HEADER_PATH")

mkdir -p build

TARGET="--target=wasm32-wasi --sysroot=${WASI_SDK_PATH}/share/wasi-sysroot"

echo "--- A compilar o servidor ---"
$CLANGPP $TARGET -O3 -I"$WAMR_INC" -c servidor.cpp -o build/servidor.o

echo "--- A compilar o wrapper de sockets WASI (C) ---"
$CLANG $TARGET -O3 -I"$WAMR_INC" -c "$WRAPPER_SRC" -o build/wasi_socket_ext.o

echo "--- A linkar ---"
$CLANGPP $TARGET \
    -Wl,--initial-memory=65536 \
    -Wl,--export=__heap_base,--export=__data_end \
    -Wl,--max-memory=65536 \
    -Wl,-z,stack-size=8192 \
    -Wl,--strip-all \
    build/servidor.o build/wasi_socket_ext.o \
    -o build/servidor.wasm

echo "--- Sucesso! ---"
