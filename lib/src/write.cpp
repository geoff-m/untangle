#include "write.h"
#include <cstring>
#include <cstdarg>
#include <platform.h>
#include <cstdio>

namespace untangle::writer {
    WriteCallback writeCb;
    void* writerState;

    constexpr auto MESSAGE_BUFFER_SIZE = 1024;
    char* messageBuffer;

    void constructWrite() {
        messageBuffer = new char[MESSAGE_BUFFER_SIZE];
        untangle_set_writer(write_stderr, nullptr);
    }

    void destroyWrite() {
        delete[] messageBuffer;
    }

    extern "C" {
    void untangle_set_writer(WriteCallback writer, void* state) {
        writeCb = writer;
        writerState = state;
    }
    }

    void write(const char* text) {
        writeCb(text, strlen(text), writerState);
    }

    void writeFormat(const char* format, ...) {
        va_list args;
        va_start(args, format);
        const auto neededLength = vsnprintf(messageBuffer, MESSAGE_BUFFER_SIZE, format, args);
        if (neededLength > MESSAGE_BUFFER_SIZE) {
            messageBuffer[MESSAGE_BUFFER_SIZE - 2] = '.';
            messageBuffer[MESSAGE_BUFFER_SIZE - 3] = '.';
            messageBuffer[MESSAGE_BUFFER_SIZE - 4] = '.';
        }
        write(messageBuffer);
        va_end(args);
    }
}
