#pragma once
#include <pthread.h>
#include <cstddef>

extern "C" {
    void untangle_set_mutex_name(pthread_mutex_t* mutex, const char* name);

    int untangle_get_mutex_name(pthread_mutex_t* mutex, char* output, int maxOutputLength);

    using WriteCallback = void (*)(const char*, size_t, void*);
    void untangle_set_writer(WriteCallback writer, void* state);
}