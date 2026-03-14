#include <csignal>
#include <thread>
#include "untangle/untangle.h"
#include <gtest/gtest.h>
#include "WaitCycle.h"

TEST(Basic, NoDeadlock) {
    std::mutex m;
    std::lock_guard l1(m);
    std::thread t([] {
        std::mutex m;
        std::lock_guard l1(m);
    });
    t.join();
}

TEST(Basic, SelfDeadlock) {
    std::mutex m;
    untangle_set_mutex_name(m.native_handle(), "m");
    std::lock_guard l1(m);
    EXPECT_EXIT(
        std::lock_guard l2(m);,
        ::testing::KilledBySignal(SIGTRAP),
        "already");
}

void mutexAndJoin() {
    pthread_setname_np(pthread_self(), "outer");
    std::mutex m;
    untangle_set_mutex_name(m.native_handle(), "m");
    std::lock_guard outerLock(m);
    std::thread t([&] {
        pthread_setname_np(pthread_self(), "inner");
        std::lock_guard innerLock(m);
    });
    t.join();
}

TEST(Basic, MutexAndJoin) {
    EXPECT_EXIT(
        mutexAndJoin();,
        ::testing::KilledBySignal(SIGTRAP),
        "2 threads by waiting for mutex \"m\"");
}

TEST(Basic, JoinSelf) {
    EXPECT_EXIT(
        pthread_join(pthread_self(), nullptr);,
        ::testing::KilledBySignal(SIGTRAP),
        "itself");
}

TEST(Basic, Cycle2) {
    EXPECT_EXIT(
        {
        WaitCycle c(2);
        c.getFirst()->join();
        },
        ::testing::KilledBySignal(SIGTRAP),
        "2 threads");
}

TEST(Basic, Cycle3) {
    EXPECT_EXIT(
        {
        WaitCycle c(3);
        c.getFirst()->join();
        },
        ::testing::KilledBySignal(SIGTRAP),
        "3 threads by");
}

TEST(Basic, Cycle4) {
    EXPECT_EXIT(
        {
        WaitCycle c(4);
        c.getFirst()->join();
        },
        ::testing::KilledBySignal(SIGTRAP),
        "4 threads and 2 mutexes by");
}

TEST(Basic, Cycle5) {
    EXPECT_EXIT(
        {
        WaitCycle c(5);
        c.getFirst()->join();
        },
        ::testing::KilledBySignal(SIGTRAP),
        "5 threads and 2 mutexes by");
}

TEST(Basic, Cycle6) {
    EXPECT_EXIT(
        {
        WaitCycle c(6);
        c.getFirst()->join();
        },
        ::testing::KilledBySignal(SIGTRAP),
        "6 threads and 3 mutexes by");
}
