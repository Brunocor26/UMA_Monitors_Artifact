#!/bin/bash
set -e

# O script deve ser executado da raiz do projeto
if [ ! -d "core" ]; then
    echo "Erro: Por favor, execute este script da raiz do projeto: ./scripts/setup.sh"
    exit 1
fi

echo "--- Inicializando sub-módulos ---"
git submodule update --init --recursive

echo "--- Aplicando rtmlib.patch ---"
# O patch agora está em scripts/ e deve ser aplicado no diretório core/
git apply --directory=core scripts/rtmlib.patch

echo "--- Pronto. O artifact está configurado. ---"
