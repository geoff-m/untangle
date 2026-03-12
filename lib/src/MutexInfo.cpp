#include "MutexInfo.h"
#include <csignal>
#include <type_traits>
#include "pthreads_intercept.h"

using namespace untangle;

MutexInfo::MutexInfo(pthread_mutex_t* wrapped)
    : wrapped(wrapped) {
}

const char* MutexInfo::get_name() const {
    return name.c_str();
}

void MutexInfo::set_name(const char* name) {
    this->name = name;
}

[[nodiscard]] std::optional<pthread_t> getThread(Awaitee awaitee) {
    switch (awaitee.index()) {
        case 0:
            return std::get<MutexInfo*>(awaitee)->get_owner();
        case 1:
            return std::get<pthread_t>(awaitee);
        default:
            __builtin_unreachable();
            return {};
    }
}

void print_thread(pthread_t thread) {
    constexpr auto MAX_NAME_LENGTH = 16;
    char name[MAX_NAME_LENGTH] = {};
    pthread_getname_np(thread, name, MAX_NAME_LENGTH);
    fprintf(stderr, "\"%s\" (%#lx)", name, thread);
}

void print_awaitee(Awaitee awaitee) {
    switch (awaitee.index()) {
        case 0:
            fprintf(stderr, "waiting for mutex %p",
                    std::get<MutexInfo*>(awaitee));
            return;
        case 1:
            const auto thread = std::get<pthread_t>(awaitee);
            fprintf(stderr, "joining thread ");
            print_thread(thread);
            return;
    }
}

std::unordered_map<pthread_t, Awaitee> untangle::waiters;

void print_deadlock(Awaitee awaitee, int threadCount, int mutexCount) {
    const auto thisThread = pthread_self();
    static_assert(std::is_integral_v<pthread_t>);
    fprintf(stderr, "Thread ");
    print_thread(thisThread);
    fprintf(stderr, " created a deadlock");
    if (threadCount > 1 || mutexCount > 1) {
        fprintf(stderr, " involving");
        if (threadCount > 1) {
            fprintf(stderr, " %d threads", threadCount);
        }
        if (mutexCount > 1) {
            if (threadCount > 1) {
                fprintf(stderr, " and");
            }
            fprintf(stderr, " %d mutexes", mutexCount);
        }
    }
    fprintf(stderr, " by ");
    print_awaitee(awaitee);
    fprintf(stderr, ":\n");
    auto threadToCheck = *getThread(awaitee);
    while (!pthread_equal(thisThread, threadToCheck)) {
        const auto nextMutexIt = waiters.find(threadToCheck);
        const auto nextAwaitee = nextMutexIt->second;
        fprintf(stderr, "    ");
        print_thread(threadToCheck);
        fprintf(stderr, " is ");
        print_awaitee(nextAwaitee);
        fprintf(stderr, ".\n");
        const auto nextThreadToCheck = *getThread(nextAwaitee);
        threadToCheck = nextThreadToCheck;
    }
    fflush(stderr);
}

void untangle::trap_if_deadlock(Awaitee awaitee) {
    const auto thisThread = pthread_self();
    int seenThreads = 0;
    int seenMutexes = 0;
    if (std::holds_alternative<MutexInfo*>(awaitee))
        ++seenMutexes;
    auto threadToCheck = getThread(awaitee);
    while (threadToCheck.has_value()) {
        ++seenThreads;
        if (pthread_equal(thisThread, *threadToCheck)) {
            print_deadlock(awaitee, seenThreads, seenMutexes);
            raise(SIGTRAP);
        }
        const auto nextMutexIt = waiters.find(threadToCheck.value());
        if (nextMutexIt == waiters.end()) {
            return;
        }
        const auto nextAwaitee = nextMutexIt->second;
        if (std::holds_alternative<MutexInfo*>(awaitee))
            ++seenMutexes;
        const auto nextThreadToCheck = getThread(nextAwaitee);
        if (!nextThreadToCheck.has_value())
            return;
        threadToCheck = nextThreadToCheck;
    }
}

int MutexInfo::lock() {
    const auto thisThread = pthread_self();
    originalFunctions.pthread_mutex_lock(&deadlockCheckMutex);
    trap_if_deadlock(this);
    waiters[thisThread] = this;
    originalFunctions.pthread_mutex_unlock(&deadlockCheckMutex);
    const auto ret = originalFunctions.pthread_mutex_lock(wrapped);
    originalFunctions.pthread_mutex_lock(&deadlockCheckMutex);
    waiters.erase(thisThread);
    owner = thisThread;
    originalFunctions.pthread_mutex_unlock(&deadlockCheckMutex);
    return ret;
}

int MutexInfo::unlock() {
    originalFunctions.pthread_mutex_lock(&deadlockCheckMutex);
    owner = {};
    originalFunctions.pthread_mutex_unlock(&deadlockCheckMutex);
    return originalFunctions.pthread_mutex_unlock(wrapped);
}

std::optional<pthread_t> MutexInfo::get_owner() const {
    return owner;
}
