#!/bin/bash

set -e

echo "=== BUILD DOCKER ==="
docker build --platform linux/386 \
    -t uma-servidor-i386 .

echo
echo "=== TAMANHO DOCKER ==="
DOCKER_SIZE=$(docker images uma-servidor-i386 \
    --format "{{.Size}}")

echo "Docker: $DOCKER_SIZE"

echo
echo "=== TAMANHO WASM ==="
ls -lh intoNuttx/wasm_server/build/servidor.wasm

echo
echo "=== BENCHMARK WASM ==="

hyperfine \
  --warmup 3 \
  --runs 20 \
  '
  iwasm --addr-pool=0.0.0.0/0 intoNuttx/wasm_server/build/servidor.wasm >/dev/null 2>&1 &
  PID=$!

  sleep 0.05

  python3 -c "
import socket
s=socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.sendto(b\"RPM=600,Frequency=100\", (\"127.0.0.1\", 5001))
s.close()
"

  sleep 0.05

  kill -9 $PID >/dev/null 2>&1 || true
  '

echo
echo
echo "=== BENCHMARK DOCKER ==="

hyperfine \
-i \
--warmup 3 \
'
CID=$(docker run \
-d \
--rm \
--platform linux/386 \
-p 5001:5001/udp \
uma-servidor-i386)

sleep 0.10

python3 -c "
import socket
s=socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.sendto(b\"RPM=600,Frequency=100\", (\"127.0.0.1\",5001))
s.close()
"

sleep 0.10

docker kill $CID >/dev/null 2>&1 || true
'

echo
echo "=== FIM ==="