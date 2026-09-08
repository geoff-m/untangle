#include "linux/pthreads-intercept.h"
#include "MutexInfo.h"
#include "platform.h"
#include <csignal>
#include <cstdio>
#include <cstring>
#include <dlfcn.h>
#include <memory>
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
static Tpthread_mutex_lock orig_lock;
int OriginalFunctions::mutex_lock(native_mutex_handle mutex) {
    return orig_lock(mutex);
}
static Tpthread_mutex_unlock orig_unlock;
int OriginalFunctions::mutex_unlock(native_mutex_handle mutex) {
    return orig_unlock(mutex);
}

static Tpthread_join orig_join;
int OriginalFunctions::thread_join(native_thread_handle thread, void** thread_return_value) {
    return orig_join(thread, thread_return_value);
}

static Tpthread_mutex_init orig_init;
int OriginalFunctions::mutex_init(native_mutex_handle mutex, const void* options) {
    return orig_init(mutex, static_cast<const pthread_mutexattr_t*>(options));
}

void OriginalFunctions::initialize() {
    orig_init = reinterpret_cast<Tpthread_mutex_init>(findNextSymbol("pthread_mutex_init"));
    //pthread_mutex_destroy = reinterpret_cast<Tpthread_mutex_destroy>(findNextSymbol("pthread_mutex_destroy"));
    orig_lock = reinterpret_cast<Tpthread_mutex_lock>(findNextSymbol("pthread_mutex_lock"));
    orig_unlock = reinterpret_cast<Tpthread_mutex_unlock>(findNextSymbol("pthread_mutex_unlock"));
    orig_join = reinterpret_cast<Tpthread_join>(findNextSymbol("pthread_join"));
}

OriginalFunctions untangle::originalFunctions;

pthread_mutex_t mutexInfosMutexValue;
native_mutex_handle untangle::mutexInfosMutex = &mutexInfosMutexValue;
std::unordered_map<pthread_mutex_t*, std::shared_ptr<MutexInfo>> mutexInfos;
pthread_mutex_t deadlockCheckMutexValue;
native_mutex_handle untangle::deadlockCheckMutex = &deadlockCheckMutexValue;

bool tryGetMutexInfo(pthread_mutex_t* mutex, std::shared_ptr<MutexInfo>& mi) {
    auto it = mutexInfos.find(mutex);
    if (it == mutexInfos.end()) {
        return false;
    }
    mi = it->second;
    return true;
}

int pthread_mutex_init(pthread_mutex_t* __mutex,
                       const pthread_mutexattr_t* __mutexattr) noexcept(true) {
    originalFunctions.mutex_lock(mutexInfosMutex);
    if (std::shared_ptr<MutexInfo> mi; tryGetMutexInfo(__mutex, mi)) [[unlikely]] {
        fprintf(stderr, "untangle: Error detected: Tried to initialize a mutex that is already initialized\n");
        raise(SIGTRAP);
    } else {
        mutexInfos.insert({__mutex, std::make_shared<MutexInfo>(__mutex)});
    }
    const auto ret = originalFunctions.mutex_init(__mutex, static_cast<const void*>(__mutexattr));;
    originalFunctions.mutex_unlock(mutexInfosMutex);
    return ret;
}

// int pthread_mutex_destroy(pthread_mutex_t* __mutex) noexcept(true) {
//     return originalFunctions.pthread_mutex_destroy(__mutex);
// }

int pthread_mutex_lock(pthread_mutex_t* __mutex) noexcept(true) {
    originalFunctions.mutex_lock(mutexInfosMutex);
    std::shared_ptr<MutexInfo> mi;
    if (!tryGetMutexInfo(__mutex, mi)) {
        // Unknown mutex.
        // Assume __mutex is a valid mutex pointer and create a new MutexInfo.
        mi = std::make_shared<MutexInfo>(__mutex);
        mutexInfos.insert({__mutex, mi});
    }
    originalFunctions.mutex_unlock(mutexInfosMutex);
    const int ret = mi->lock();
    return ret;
}

int pthread_mutex_unlock(pthread_mutex_t* __mutex) noexcept(true) {
    originalFunctions.mutex_lock(mutexInfosMutex);
    int ret;
    if (std::shared_ptr<MutexInfo> mi; tryGetMutexInfo(__mutex, mi)) {
        ret = mi->unlock();
    } else {
        // Unknown mutex.
        // Fall back to unwrapped handling.
        ret = originalFunctions.mutex_unlock(__mutex);
    }
    originalFunctions.mutex_unlock(mutexInfosMutex);
    return ret;
}

int pthread_join(pthread_t __th, void** __thread_return) {
    const auto thisThread = pthread_self();
    originalFunctions.mutex_lock(deadlockCheckMutex);
    waiters[thisThread] = __th;
    trap_if_deadlock(__th);
    originalFunctions.mutex_unlock(deadlockCheckMutex);
    const auto ret = originalFunctions.thread_join(__th, __thread_return);
    waiters.erase(thisThread);
    return ret;
}

extern "C" {
void untangle_set_mutex_name(pthread_mutex_t* mutex, const char* name) {
    originalFunctions.mutex_lock(mutexInfosMutex);
    std::shared_ptr<MutexInfo> mi;
    if (!tryGetMutexInfo(mutex, mi)) {
        mutexInfos.insert({mutex, mi = std::make_shared<MutexInfo>(mutex)});
    }
    mi->set_name(name);
    originalFunctions.mutex_unlock(mutexInfosMutex);
}

int untangle_get_mutex_name(pthread_mutex_t* mutex, char* output, int maxOutputLength) {
    originalFunctions.mutex_lock(mutexInfosMutex);
    int ret;
    std::shared_ptr<MutexInfo> mi;
    if (tryGetMutexInfo(mutex, mi)) {
        const auto actualLength = static_cast<int>(mi->get_name().size() + 1); // add 1 for null terminator
        const auto actualEnd = std::min(actualLength, maxOutputLength) - 1;
        std::memcpy(output, mi->get_name().c_str(), actualEnd);
        output[actualEnd] = 0;
        ret = actualLength;
    } else {
        ret = -1;
    }
    originalFunctions.mutex_unlock(mutexInfosMutex);
    return ret;
}
}
