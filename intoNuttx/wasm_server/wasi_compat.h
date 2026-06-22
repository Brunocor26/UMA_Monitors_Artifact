/**
 * wasi_compat.h - Stubs para APIs POSIX indisponíveis em WASI.
 *
 * O cabeçalho do monitor inclui periodicmonitor.h (rtmlib), que arrasta
 * task_compat.h; o WASI não tem escalonamento, por isso fornecemos no-ops
 * para que compile. O caminho do monitor periódico não é usado em runtime.
 */
#pragma once

#include <pthread.h>

#ifndef YIELD
#define YIELD() do {} while(0)
#endif

#ifndef pthread_attr_setschedparam
static inline int pthread_attr_setschedparam(pthread_attr_t *attr,
                                             const struct sched_param *param) {
    (void)attr; (void)param;
    return 0;
}
#endif
