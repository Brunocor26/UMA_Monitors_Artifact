/**
 * wasi_stubs.cpp — Weak stub implementations for POSIX scheduling APIs
 * that are declared in WASI's pthread.h but not implemented in wasi-libc.
 */
#include <pthread.h>
#include <sched.h>

extern "C" {

//se nao houver outra definicao mais forte, esta fica
__attribute__((weak))
int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy) {
    (void)attr; (void)policy;
    return 0;
}

} // extern "C"
