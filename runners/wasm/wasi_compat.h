/**
 * wasi_compat.h — Stubs for POSIX APIs unavailable in WASI.
 *
 * WASI has no CPU-scheduling support, so pthread_attr_setschedparam is not
 * declared in its pthread.h.  We provide a no-op macro so that rtmlib's
 * task_compat.h compiles cleanly; the periodic-monitor path is never called
 * at runtime in this test.
 */
#pragma once

#include <pthread.h>

/* atomic_compat.h only defines YIELD() inside the __x86_64__ sub-branch;
 * define it as a no-op for the __i386__ (32-bit) path used on wasm32. */
#ifndef YIELD
#define YIELD() do {} while(0)
#endif

#ifndef pthread_attr_setschedparam
static inline int pthread_attr_setschedparam(pthread_attr_t *attr,
                                             const struct sched_param *param) {
    (void)attr; (void)param;
    return 0; /* no-op: WASI has no scheduling support */
}
#endif

