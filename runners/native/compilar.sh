#!/bin/bash
# Compilador C++17 com os cabeçalhos do rtmlib e do monitor gerado.
g++ -std=c++17 \
    -I../../core/src \
    -I../../monitors/duration_formula/headers \
    main.cpp -o monitor -lpthread -latomic

echo "--- Binário 'monitor' gerado. Para rodar: ./monitor ---"
