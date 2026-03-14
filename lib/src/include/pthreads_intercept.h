#pragma once
#include <pthread.h>

int pthread_mutex_init(pthread_mutex_t* __mutex,
                       const pthread_mutexattr_t* __mutexattr) noexcept(true);

int pthread_mutex_destroy(pthread_mutex_t* __mutex) noexcept(true);

int pthread_mutex_lock(pthread_mutex_t* __mutex) noexcept(true);

int pthread_mutex_unlock(pthread_mutex_t* __mutex) noexcept(true);

int pthread_join(pthread_t __th, void** __thread_return);

namespace untangle {
    struct OriginalFunctions {
        using Tpthread_mutex_init = int (*)(pthread_mutex_t*, const pthread_mutexattr_t*);
        using Tpthread_mutex_destroy = int (*)(pthread_mutex_t*);
        using Tpthread_mutex_lock = int (*)(pthread_mutex_t*);
        using Tpthread_mutex_unlock = int (*)(pthread_mutex_t*);
        using Tpthread_join = int (*)(pthread_t, void**);

        void initialize();
        Tpthread_mutex_init pthread_mutex_init = nullptr;
        Tpthread_mutex_destroy pthread_mutex_destroy = nullptr;
        Tpthread_mutex_lock pthread_mutex_lock = nullptr;
        Tpthread_mutex_unlock pthread_mutex_unlock = nullptr;
        Tpthread_join pthread_join = nullptr;
    };
    extern OriginalFunctions originalFunctions;

    extern pthread_mutex_t deadlockCheckMutex;
}
