#!/bin/bash

# Verifica se um ficheiro foi passado como argumento
if [ -z "$1" ]; then
    echo "Erro: Forneça o nome do ficheiro de entrada."
    echo "Uso: ./run.sh meu_programa.cpp"
    exit 1
fi

# Variáveis de entrada e saída
INPUT_FILE=$1
# Extrai o nome base (ex: main.cpp -> main)
BASENAME=$(basename "$INPUT_FILE" | cut -d. -f1)
OUTPUT_FILE="wasm/${BASENAME}.wasm"

# Garante que a pasta de output existe
mkdir -p wasm

# Comando de compilação
/opt/wasi-sdk/bin/clang++ \
  --target=wasm32-wasi \
  --sysroot=/opt/wasi-sdk/share/wasi-sysroot \
  -Wall -Wextra -std=gnu++11 -DRTMLIB_ENABLE_MAP_SORT \
  -D__i386__ -D__x86__ \
  -Duseconds_t=uint32_t \
  -fno-exceptions \
  -include wasi_compat.h \
  -I"$(pwd)/../rtmlib/src" \
  "$INPUT_FILE" wasi_stubs.cpp -o "$OUTPUT_FILE"

# Verifica se a compilação teve sucesso
if [ $? -eq 0 ]; then
    echo "Sucesso! Gerado: $OUTPUT_FILE"
else
    echo "Erro na compilação."
fi