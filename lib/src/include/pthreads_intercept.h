#pragma once
#include <pthread.h>

int pthread_mutex_lock(pthread_mutex_t* __mutex) noexcept(true);

int pthread_join(pthread_t __th, void** __thread_return);
