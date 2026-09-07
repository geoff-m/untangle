#pragma once

#if defined(_WIN32)
#define UNTANGLE_EXPORT __declspec(dllexport)
#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif
using native_thread_handle = uintptr_t;
using native_mutex_handle = uintptr_t;
#else
#if defined(__GNUC__)
#include <pthread.h>
#define UNTANGLE_EXPORT __attribute__((__visibility__("default")))
using native_thread_handle = pthread_t;
using native_mutex_handle = pthread_mutex_t*;
#endif
#endif

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif

    UNTANGLE_EXPORT
    void untangle_set_mutex_name(native_mutex_handle mutex, const char* name);

    UNTANGLE_EXPORT
    int untangle_get_mutex_name(native_mutex_handle mutex, char* output, int maxOutputLength);

    UNTANGLE_EXPORT
    void untangle_set_writer(void (*writer)(const char*, size_t, void*), void* state);

#ifdef __cplusplus
}
#endif