#include "MutexInfo.h"
#include <csignal>
#include <type_traits>
#include "pthreads_intercept.h"
#include "write.h"

using namespace untangle;

MutexInfo::MutexInfo(pthread_mutex_t* wrapped)
    : wrapped(wrapped) {
}

std::string MutexInfo::get_name() const {
    return name;
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
    writeFormat("\"%s\" (%#lx)", name, thread);
}

void print_awaitee(Awaitee awaitee) {
    switch (awaitee.index()) {
        case 0: {
            auto* mi = std::get<MutexInfo*>(awaitee);
            auto mutexName = mi->get_name();
            if (mutexName.empty())
                writeFormat("waiting for mutex %p", mi->get_wrapped());
            else
                writeFormat("waiting for mutex \"%s\" (%p)", mutexName.c_str(), mi->get_wrapped());
            return;
        }
        case 1: {
            const auto thread = std::get<pthread_t>(awaitee);
            write("joining thread ");
            print_thread(thread);
            return;
        }
        default: {
            __builtin_unreachable();
            return;
        }
    }
}

void print_owner_if_mutex(Awaitee awaitee) {
    if (std::holds_alternative<MutexInfo*>(awaitee)) {
        write(", which is held by thread ");
        print_thread(std::get<MutexInfo*>(awaitee)->get_owner().value());
    }
}

std::unordered_map<pthread_t, Awaitee> untangle::waiters;

void print_deadlock(Awaitee awaitee, int threadCount, int mutexCount) {
    const auto thisThread = pthread_self();
    static_assert(std::is_integral_v<pthread_t>);
    write("untangle: Thread ");
    print_thread(thisThread);
    write(" created a deadlock");
    if (threadCount > 1 || mutexCount > 1) {
        write(" involving");
        if (threadCount > 1) {
            writeFormat(" %d threads", threadCount);
        }
        if (mutexCount > 1) {
            if (threadCount > 1) {
                write(" and");
            }
            writeFormat(" %d mutexes", mutexCount);
        }
    }
    write(" by ");
    switch (awaitee.index()) {
        case 0: {
            const auto owner = std::get<MutexInfo*>(awaitee)->get_owner().value();
            print_awaitee(awaitee);
            if (pthread_equal(thisThread, owner)) {
                write(", which it already holds.\n");
                return;
            } else {
                write(", which is held by thread ");
                print_thread(owner);
            }
            break;
        }
        case 1: {
            const auto otherThread = std::get<pthread_t>(awaitee);
            if (pthread_equal(thisThread, otherThread)) {
                write("joining itself.\n");
                return;
            } else {
                write("joining thread ");
                print_thread(otherThread);
                break;
            }
        }
    }
    write(":\n");
    auto threadToCheck = *getThread(awaitee);
    while (!pthread_equal(thisThread, threadToCheck)) {
        const auto nextMutexIt = waiters.find(threadToCheck);
        const auto nextAwaitee = nextMutexIt->second;
        write("untangle:  Thread ");
        print_thread(threadToCheck);
        write(" is ");
        print_awaitee(nextAwaitee);
        print_owner_if_mutex(nextAwaitee);
        write(".\n");
        const auto nextThreadToCheck = *getThread(nextAwaitee);
        threadToCheck = nextThreadToCheck;
    }
    fflush(stderr);
}

void untangle::trap_if_deadlock(Awaitee awaitee) {
    const auto thisThread = pthread_self();
    int seenThreads = 0;
    int seenMutexes = 0;
    auto threadToCheck = getThread(awaitee);
    if (std::holds_alternative<MutexInfo*>(awaitee))
        ++seenMutexes;
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
        if (std::holds_alternative<MutexInfo*>(nextAwaitee))
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

pthread_mutex_t* MutexInfo::get_wrapped() const {
    return wrapped;
}