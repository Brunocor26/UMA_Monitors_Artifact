/**
 * wasi_stubs.cpp - Stubs fracos para APIs POSIX de escalonamento declaradas
 * no pthread.h do WASI mas não implementadas pela wasi-libc.
 */
#include <pthread.h>
#include <sched.h>

extern "C" {

__attribute__((weak))
int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy) {
    (void)attr; (void)policy;
    return 0;
}

} // extern "C"
