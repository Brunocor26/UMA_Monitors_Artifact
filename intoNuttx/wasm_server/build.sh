#!/bin/bash
set -e

WASI_SDK_PATH="/Users/brunocor/Desktop/wasi-sdk-33.0-x86_64-macos"
CLANG="${WASI_SDK_PATH}/bin/clang"
CLANGPP="${WASI_SDK_PATH}/bin/clang++"

# Mudar de acordo
WAMR_ROOT="/Users/brunocor/Desktop/wasm-micro-runtime"
HEADER_PATH=$(find "$WAMR_ROOT" -name "wasi_socket_ext.h" | head -n 1)
WRAPPER_SRC=$(find "$WAMR_ROOT" -name "wasi_socket_ext.c" | head -n 1)
WAMR_INC=$(dirname "$HEADER_PATH")

MON_ROOT="../../monitors"

mkdir -p build

TARGET="--target=wasm32-wasi --sysroot=${WASI_SDK_PATH}/share/wasi-sysroot"

# Flags para as TUs C++ que usam a rtmlib (monitor).
CXXFLAGS="-O3 -std=gnu++17 -D__i386__ -D__x86__ -Duseconds_t=uint32_t \
    -fno-exceptions -include wasi_compat.h \
    -I../../rtmlib/src"

# build_monitor <monitor-dir> <func-prefix> <has-prop-b>
# Descobre o hash a partir do nome de Rtm_compute_*.h (muda a cada regeneração)
# e compila o template genérico para esse monitor.
build_monitor()
{
    local dir="$MON_ROOT/$1"
    local prefix="$2"
    local hasb="$3"

    local comp
    comp=$(ls "$dir"/Rtm_compute_*.h 2>/dev/null | head -n 1)
    [ -z "$comp" ] && comp=$(ls "$dir"/headers/Rtm_compute_*.h 2>/dev/null | head -n 1)
    if [ -z "$comp" ]; then
        echo "ERRO: nenhum Rtm_compute_*.h em $dir"
        exit 1
    fi

    local hdrdir base hash hashu inst mon
    hdrdir=$(dirname "$comp")
    base=$(basename "$comp" .h)        # Rtm_compute_<hash>
    hash=${base#Rtm_compute_}
    hashu=$(echo "$hash" | tr '[:lower:]' '[:upper:]')
    inst="$hdrdir/Rtm_instrument_$hash.h"
    mon="$hdrdir/Rtm_monitor_$hash.h"

    echo "  - $1: hash=$hash prefix=$prefix has_b=$hasb"

    $CLANGPP $TARGET $CXXFLAGS \
        -I"$hdrdir" \
        -DMON_H=$hash -DMON_HU=$hashu \
        -DMON_PREFIX=$prefix -DHAS_PROP_B=$hasb \
        -DMON_COMPUTE_HDR="\"$comp\"" \
        -DMON_INSTRUMENT_HDR="\"$inst\"" \
        -DMON_MONITOR_HDR="\"$mon\"" \
        -c mon_template.cpp -o "build/$prefix.o"
}

echo "--- A compilar o monitor (hash auto-descoberto) ---"
build_monitor final_monitor mon 1

echo "--- A compilar o servidor ---"
$CLANGPP $TARGET -O3 -I"$WAMR_INC" -c servidor.cpp   -o build/servidor.o
$CLANGPP $TARGET $CXXFLAGS                -c wasi_stubs.cpp -o build/wasi_stubs.o

echo "--- A compilar o wrapper de sockets WASI (C) ---"
$CLANG $TARGET -O3 -I"$WAMR_INC" -c "$WRAPPER_SRC" -o build/wasi_socket_ext.o

echo "--- A linkar ---"
$CLANGPP $TARGET \
    -Wl,--initial-memory=65536 \
    -Wl,--export=__heap_base,--export=__data_end \
    -Wl,--max-memory=65536 \
    -Wl,-z,stack-size=8192 \
    -Wl,--strip-all \
    build/servidor.o build/mon.o build/wasi_stubs.o build/wasi_socket_ext.o \
    -o build/servidor.wasm

echo "--- Sucesso! ---"
