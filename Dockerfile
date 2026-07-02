# 1- Build
FROM i386/alpine:latest AS builder

RUN apk add --no-cache \
    build-base \
    bash

WORKDIR /app

COPY . .

WORKDIR /app/intoNuttx/wasm_server

RUN COMP=$(find ../../monitors/final_monitor -name "Rtm_compute_*.h" | head -n 1) && \
    [ -n "$COMP" ] && \
    HDRDIR=$(dirname "$COMP") && \
    HASH=$(basename "$COMP" .h | sed 's/Rtm_compute_//') && \
    HASHU=$(echo "$HASH" | tr '[:lower:]' '[:upper:]') && \
    echo "Hash detetado: $HASH" && \
    \
    g++ \
      -O3 \
      -std=gnu++17 \
      -fpermissive \
      -fno-exceptions \
      -D__i386__ \
      -D__x86__ \
      -Duseconds_t=uint32_t \
      '-DYIELD()=0' \
      -I../../rtmlib/src \
      -I"$HDRDIR" \
      -DMON_H=$HASH \
      -DMON_HU=$HASHU \
      -DMON_PREFIX=mon \
      -DHAS_PROP_B=1 \
      -DMON_COMPUTE_HDR='"'${COMP}'"' \
      -DMON_INSTRUMENT_HDR='"'$HDRDIR/Rtm_instrument_$HASH.h'"' \
      -DMON_MONITOR_HDR='"'$HDRDIR/Rtm_monitor_$HASH.h'"' \
      -c mon_template.cpp \
      -o mon.o && \
    \
    g++ \
      -O3 \
      -c servidor.cpp \
      -o servidor.o && \
    \
    g++ \
      mon.o \
      servidor.o \
      -static-libstdc++ \
      -static-libgcc \
      -o servidor_nativo \
      -s



# 2 — Runtime

FROM i386/alpine:latest

WORKDIR /app

COPY --from=builder \
    /app/intoNuttx/wasm_server/servidor_nativo \
    .

EXPOSE 5001/udp

CMD ["./servidor_nativo"]