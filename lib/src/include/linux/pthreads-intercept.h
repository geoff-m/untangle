#pragma once
#include <pthread.h>

extern "C" {
int pthread_mutex_init(pthread_mutex_t* __mutex,
                       const pthread_mutexattr_t* __mutexattr) noexcept(true);

int pthread_mutex_destroy(pthread_mutex_t* __mutex) noexcept(true);

int pthread_mutex_lock(pthread_mutex_t* __mutex) noexcept(true);

int pthread_mutex_unlock(pthread_mutex_t* __mutex) noexcept(true);

int pthread_join(pthread_t __th, void** __thread_return);
}

using Tpthread_mutex_init = int (*)(pthread_mutex_t*, const pthread_mutexattr_t*);
using Tpthread_mutex_destroy = int (*)(pthread_mutex_t*);
using Tpthread_mutex_lock = int (*)(pthread_mutex_t*);
using Tpthread_mutex_unlock = int (*)(pthread_mutex_t*);
using Tpthread_join = int (*)(pthread_t, void**);
