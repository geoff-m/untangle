#include "pthreads_intercept.h"
#include "MutexInfo.h"
#include <csignal>
#include <cstdio>
#include <cstring>
#include <dlfcn.h>
#include <unistd.h>
#include <unordered_map>


void* findNextSymbol(const char* originalName) {
    (void)dlerror(); // Clear error.
    auto* ret = dlsym(RTLD_NEXT, originalName);
    const auto error = dlerror();
    if (error) {
        fprintf(stderr, "untangle failed to get address of %s: %s\n", originalName, error);
    }
    return ret;
}

using namespace untangle;

void OriginalFunctions::initialize() {
    pthread_mutex_init = reinterpret_cast<Tpthread_mutex_init>(findNextSymbol("pthread_mutex_init"));
    pthread_mutex_destroy = reinterpret_cast<Tpthread_mutex_destroy>(findNextSymbol("pthread_mutex_destroy"));
    pthread_mutex_lock = reinterpret_cast<Tpthread_mutex_lock>(findNextSymbol("pthread_mutex_lock"));
    pthread_mutex_unlock = reinterpret_cast<Tpthread_mutex_unlock>(findNextSymbol("pthread_mutex_unlock"));
    pthread_join = reinterpret_cast<Tpthread_join>(findNextSymbol("pthread_join"));
}

OriginalFunctions untangle::originalFunctions;

pthread_mutex_t mutexInfosMutex;
std::unordered_map<pthread_mutex_t*, MutexInfo*> mutexInfos;
pthread_mutex_t untangle::deadlockCheckMutex;

bool tryGetMutexInfo(pthread_mutex_t* mutex, MutexInfo** mi) {
    auto it = mutexInfos.find(mutex);
    if (it == mutexInfos.end()) {
        return false;
    }
    *mi = it->second;
    return true;
}

int pthread_mutex_init(pthread_mutex_t* __mutex,
                       const pthread_mutexattr_t* __mutexattr) noexcept(true) {
    originalFunctions.pthread_mutex_lock(&mutexInfosMutex);
    if (MutexInfo* mi; tryGetMutexInfo(__mutex, &mi)) [[unlikely]] {
        fprintf(stderr, "untangle: Error detected: Tried to initialize a mutex that is already initialized\n");
        raise(SIGTRAP);
    } else {
        mutexInfos.insert({__mutex, new MutexInfo(__mutex)});
    }
    const auto ret = originalFunctions.pthread_mutex_init(__mutex, __mutexattr);;
    originalFunctions.pthread_mutex_unlock(&mutexInfosMutex);
    return ret;
}

int pthread_mutex_destroy(pthread_mutex_t* __mutex) noexcept(true) {
    return originalFunctions.pthread_mutex_destroy(__mutex);
}

int pthread_mutex_lock(pthread_mutex_t* __mutex) noexcept(true) {
    originalFunctions.pthread_mutex_lock(&mutexInfosMutex);
    MutexInfo* mi;
    if (!tryGetMutexInfo(__mutex, &mi)) {
        // Unknown mutex.
        // Assume __mutex is a valid mutex pointer and create a new MutexInfo.
        mi = new MutexInfo(__mutex);
        mutexInfos.insert({__mutex, mi});
    }
    originalFunctions.pthread_mutex_unlock(&mutexInfosMutex);
    const int ret = mi->lock();
    return ret;
}

int pthread_mutex_unlock(pthread_mutex_t* __mutex) noexcept(true) {
    originalFunctions.pthread_mutex_lock(&mutexInfosMutex);
    int ret;
    if (MutexInfo* mi; tryGetMutexInfo(__mutex, &mi)) {
        ret = mi->unlock();
    } else {
        // Unknown mutex.
        // Fall back to unwrapped handling.
        ret = originalFunctions.pthread_mutex_unlock(__mutex);
    }
    originalFunctions.pthread_mutex_unlock(&mutexInfosMutex);
    return ret;
}

int pthread_join(pthread_t __th, void** __thread_return) {
    const auto thisThread = pthread_self();
    originalFunctions.pthread_mutex_lock(&deadlockCheckMutex);
    waiters[thisThread] = __th;
    trap_if_deadlock(__th);
    originalFunctions.pthread_mutex_unlock(&deadlockCheckMutex);
    const auto ret = originalFunctions.pthread_join(__th, __thread_return);
    waiters.erase(thisThread);
    return ret;
}

extern "C" {
void untangle_set_mutex_name(pthread_mutex_t* mutex, const char* name) {
    originalFunctions.pthread_mutex_lock(&mutexInfosMutex);
    MutexInfo* mi;
    if (!tryGetMutexInfo(mutex, &mi)) {
        mutexInfos.insert({mutex, mi = new MutexInfo(mutex)});
    }
    mi->set_name(name);
    originalFunctions.pthread_mutex_unlock(&mutexInfosMutex);
}

int untangle_get_mutex_name(pthread_mutex_t* mutex, char* output, int maxOutputLength) {
    originalFunctions.pthread_mutex_lock(&mutexInfosMutex);
    int ret;
    MutexInfo* mi;
    if (tryGetMutexInfo(mutex, &mi)) {
        const auto actualLength = static_cast<int>(mi->get_name().size() + 1); // add 1 for null terminator
        const auto actualEnd = std::min(actualLength, maxOutputLength) - 1;
        std::memcpy(output, mi->get_name().c_str(), actualEnd);
        output[actualEnd] = 0;
        ret = actualLength;
    } else {
        ret = -1;
    }
    originalFunctions.pthread_mutex_unlock(&mutexInfosMutex);
    return ret;
}
}

__attribute__((constructor))
void initialize() {
    originalFunctions.initialize();
    originalFunctions.pthread_mutex_init(&mutexInfosMutex, nullptr);
    originalFunctions.pthread_mutex_init(&deadlockCheckMutex, nullptr);
}
