#include <gtest/gtest.h>
#include "untangle/untangle.h"

TEST(MutexName, SetGet) {
    std::mutex m;
    const char NAME[] = "afji32ty98";
    untangle_set_mutex_name(m.native_handle(), NAME);
    char output[100] = {};
    const auto actualLength = untangle_get_mutex_name(m.native_handle(), output, 100);
    EXPECT_EQ(sizeof(NAME), actualLength);
    EXPECT_STREQ(NAME, output);
}

TEST(MutexName, SetGetTooShort) {
    std::mutex m;
    const char NAME[] = "123456789";
    untangle_set_mutex_name(m.native_handle(), NAME);
    char output[5] = {};
    const auto actualLength = untangle_get_mutex_name(m.native_handle(), output, 5);
    EXPECT_EQ(sizeof(NAME), actualLength);
    EXPECT_STREQ("1234", output);
}

TEST(MutexName, LockTwice) {
    std::mutex m;
    const char NAME[] = "afji32ty98";
    untangle_set_mutex_name(m.native_handle(), NAME);
    EXPECT_EXIT(
        {
        m.lock();
        m.lock();
        },
        ::testing::KilledBySignal(SIGTRAP),
        NAME);
}
