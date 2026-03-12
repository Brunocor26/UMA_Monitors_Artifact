# WASM Build — Changes and Command

## Created files

### `wasi_compat.h`
Header injected via `-include` before any other file.
Defines two items that the WASI SDK does not provide:

- **`YIELD()`** — no-op macro. `atomic_compat.h` only defines `YIELD()` in
  the `__x86_64__` branch; in the `__i386__` branch (used for wasm32) the macro
  is left undefined and compilation fails.
- **`pthread_attr_setschedparam`** — inline stub that returns 0. WASI guards
  this function behind `#ifdef __wasilibc_unmodified_upstream` because it has no
  CPU scheduling support, so it is never declared.

### `wasi_stubs.cpp`
Implementation file compiled alongside `main.cpp`.
Provides a `__attribute__((weak))` stub for:

- **`pthread_attr_setschedpolicy`** — declared in WASI's `pthread.h` but not
  implemented in `wasi-libc`; without this stub the linker fails with "undefined symbol".

---

## Why not use `-D__HW__`

The original `-D__HW__` flag triggers a `#error` directly in the preprocessor
inside `writer.h` (the `#if defined(__HW__)` block), preventing compilation
regardless of which functions are actually called.

---

## Compilation command

```bash
/opt/wasi-sdk/bin/clang++ \
  --target=wasm32-wasi \
  --sysroot=/opt/wasi-sdk/share/wasi-sysroot \
  -Wall -Wextra -std=gnu++11 -DRTMLIB_ENABLE_MAP_SORT \
  -D__i386__ -D__x86__ \
  -Duseconds_t=uint32_t \
  -fno-exceptions \
  -include wasi_compat.h \
  -I"$(pwd)/../../rtmlib/src" \
  main.cpp wasi_stubs.cpp -o wasm/cpptest.wasm
```

### New flags relative to the original command

| Flag | Reason |
|------|--------|
| `--target=wasm32-wasi` | Standard target without shared memory; `wasm32-wasi-threads` enabled shared memory which `iwasm` does not support by default |
| `-D__i386__ -D__x86__` | Selects the 32-bit branch in `atomic_compat.h` and `time_compat.h`; the `__x86_64__` branch would use `__int128` (16 bytes), which wasm32 does not support |
| `-Duseconds_t=uint32_t` | The WASI sysroot provides `suseconds_t` but not `useconds_t` |
| `-fno-exceptions` | Avoids unresolved `__cxa_*` symbols |
| `-include wasi_compat.h` | Injects the stubs above before any library header |
| `wasi_stubs.cpp` | Weak implementation of `pthread_attr_setschedpolicy` |
