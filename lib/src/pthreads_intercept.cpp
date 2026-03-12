#include "pthreads_intercept.h"

#include <cstdio>
#include <dlfcn.h>

using TOriginal_pthread_mutex_lock = int (*)(pthread_mutex_t*);
TOriginal_pthread_mutex_lock original_pthread_mutex_lock;

using TOriginal_pthread_join = int (*)(pthread_t, void**);
TOriginal_pthread_join original_pthread_join;

void* findNextSymbol(const char* originalName) {
    (void)dlerror(); // Clear error.
    auto* ret = dlsym(RTLD_NEXT, originalName);
    const auto error = dlerror();
    if (error) {
        fprintf(stderr, "untangle failed to get address of %s: %s\n", originalName, error);
    }
    return ret;
}

__attribute__((constructor))
void initialize() {
    original_pthread_mutex_lock = reinterpret_cast<TOriginal_pthread_mutex_lock>(findNextSymbol("pthread_mutex_lock"));
    original_pthread_join = reinterpret_cast<TOriginal_pthread_join>(findNextSymbol("pthread_join"));
}

int pthread_mutex_lock(pthread_mutex_t* __mutex) noexcept(true) {
    printf("Hello from pthread_mutex_lock\n");
    return original_pthread_mutex_lock(__mutex);
}

int pthread_join(pthread_t __th, void** __thread_return) {
    printf("Hello from pthread_join\n");
    return original_pthread_join(__th, __thread_return);
}
