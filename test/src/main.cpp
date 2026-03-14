#include <gtest/gtest.h>

int main(int argc, char** argv) {
    //GTEST_FLAG_SET(death_test_style, "threadsafe");
    GTEST_FLAG_SET(death_test_style, "fast");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}