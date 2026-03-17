#pragma once
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>

namespace untangle {
    class MutexInfo {
        std::string name;
        pthread_mutex_t* wrapped;
        std::optional<pthread_t> owner;

    public:
        explicit MutexInfo(pthread_mutex_t* wrapped);

        void set_name(const char* name);

        [[nodiscard]] std::string get_name() const;

        int lock();

        int unlock();

        [[nodiscard]] std::optional<pthread_t> get_owner() const;

        [[nodiscard]] pthread_mutex_t* get_wrapped() const;
    };

    using Awaitee = std::variant<MutexInfo*, pthread_t>;

    // Caller should hold deadlockCheckMutex.
    void trap_if_deadlock(Awaitee awaitee);

    // thread x awaited thing.
    // guarded by deadlockCheckMutex.
    extern std::unordered_map<pthread_t, Awaitee> waiters;

}

