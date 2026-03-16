#pragma once
#include <pthread.h>

#define UNTANGLE_EXPORT __attribute__((__visibility__("default")))

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif

    UNTANGLE_EXPORT
    void untangle_set_mutex_name(pthread_mutex_t* mutex, const char* name);

    UNTANGLE_EXPORT
    int untangle_get_mutex_name(pthread_mutex_t* mutex, char* output, int maxOutputLength);

    UNTANGLE_EXPORT
    void untangle_set_writer(void (*writer)(const char*, size_t, void*), void* state);

#ifdef __cplusplus
}
#endif