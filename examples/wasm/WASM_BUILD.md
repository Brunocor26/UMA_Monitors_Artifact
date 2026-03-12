# Build para WASM — alterações e comando

## Ficheiros criados

### `wasi_compat.h`
Header incluído via `-include` antes de qualquer outro ficheiro.
Define dois itens que o WASI SDK não fornece:

- **`YIELD()`** — macro no-op. O `atomic_compat.h` só define `YIELD()` no
  sub-ramo `__x86_64__`; no ramo `__i386__` (usado para wasm32) a macro fica
  indefinida e a compilação falha.
- **`pthread_attr_setschedparam`** — stub inline que retorna 0. O WASI guarda
  esta função atrás de `#ifdef __wasilibc_unmodified_upstream` porque não tem
  suporte a scheduling de CPU, por isso nunca é declarada.

### `wasi_stubs.cpp`
Ficheiro de implementação compilado junto com `main.cpp`.
Fornece um stub com `__attribute__((weak))` para:

- **`pthread_attr_setschedpolicy`** — declarada no `pthread.h` do WASI mas não
  implementada em `wasi-libc`; sem este stub o linker falha com "undefined symbol".

---

## Porquê não usar `-D__HW__`

A flag original `-D__HW__` causa um `#error` directo no pré-processador dentro
de `writer.h` (bloco `#if defined(__HW__)`), impedindo a compilação
independentemente das funções que sejam efectivamente chamadas.

---

## Comando de compilação

```bash
/opt/wasi-sdk/bin/clang++ \
  --target=wasm32-wasi \
  --sysroot=/opt/wasi-sdk/share/wasi-sysroot \
  -Wall -Wextra -std=gnu++11 -DRTMLIB_ENABLE_MAP_SORT \
  -D__i386__ -D__x86__ \
  -Duseconds_t=uint32_t \
  -fno-exceptions \
  -include wasi_compat.h \
  -I"$(pwd)/../rtmlib/src" \
  main.cpp wasi_stubs.cpp -o wasm/cpptest.wasm
```

### Flags novas em relação ao comando original

| Flag | Motivo |
|------|--------|
| `--target=wasm32-wasi` | Target standard sem memória partilhada; `wasm32-wasi-threads` activava shared memory que o `iwasm` não suporta por defeito |
| `-D__i386__ -D__x86__` | Selecciona o ramo de 32 bits em `atomic_compat.h` e `time_compat.h`; o ramo `__x86_64__` usaria `__int128` (16 bytes) que o wasm32 não suporta |
| `-Duseconds_t=uint32_t` | O sysroot do WASI tem `suseconds_t` mas não `useconds_t` |
| `-fno-exceptions` | Evita símbolos `__cxa_*` não resolvidos |
| `-include wasi_compat.h` | Injeta as stubs acima antes de qualquer header da biblioteca |
| `wasi_stubs.cpp` | Implementação fraca de `pthread_attr_setschedpolicy` |
