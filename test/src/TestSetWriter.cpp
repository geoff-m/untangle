#include <gtest/gtest.h>
#include "OutputCapture.h"
#include "untangle/untangle.h"

void captureToUpper(const char* text, size_t length, void*) {
    for (size_t i = 0; i < length; ++i) {
        char c = text[i];
        if (c >= 'a' && c <= 'z') {
            c += 'A' - 'a';
        }
        fprintf(stderr, "%c", c);
    }
    fflush(stderr);
}

TEST(SetWriter, ToUpper) {
    EXPECT_EXIT(
        {
        std::mutex m;
        OutputCapture out;
        pthread_setname_np(pthread_self(), "qegiwbfuas");
        untangle_set_writer(captureToUpper, &out);
        m.lock();
        m.lock();
        },
        ::testing::KilledBySignal(SIGTRAP),
        "QEGIWBFUAS");
}
