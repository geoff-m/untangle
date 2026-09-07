#pragma once
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include "untangle/untangle.h"

namespace untangle {
    class MutexInfo {
        std::string name;
        native_mutex_handle wrapped;
        std::optional<native_thread_handle> owner;

    public:
        explicit MutexInfo(native_mutex_handle wrapped);

        void set_name(const char* name);

        [[nodiscard]] std::string get_name() const;

        int lock();

        int unlock();

        [[nodiscard]] std::optional<native_thread_handle> get_owner() const;

        [[nodiscard]] native_mutex_handle get_wrapped() const;
    };

    using Awaitee = std::variant<MutexInfo*, native_thread_handle>;

    // Caller should hold deadlockCheckMutex.
    void trap_if_deadlock(Awaitee awaitee);

    // thread x awaited thing.
    // guarded by deadlockCheckMutex.
    extern std::unordered_map<native_thread_handle, Awaitee> waiters;

}

