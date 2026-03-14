#include "write.h"
#include <cstring>
#include <cstdarg>
#include <unistd.h>
#include <cstdio>

WriteCallback untangle::writer;
void* untangle::writerState;

constexpr auto MESSAGE_BUFFER_SIZE = 1024;
char* messageBuffer;

void defaultWriter(const char* text, size_t length, void*) {
    ::write(STDERR_FILENO, text, length);
}

__attribute__((constructor))
void constructWrite() {
    messageBuffer = new char[MESSAGE_BUFFER_SIZE];
    untangle_set_writer(defaultWriter, nullptr);
}

__attribute__((destructor))
void destroyWrite() {
    delete[] messageBuffer;
}

extern "C" {
void untangle_set_writer(WriteCallback writer, void* state) {
    untangle::writer = writer;
    untangle::writerState = state;
}
}

void untangle::write(const char* text) {
    writer(text, strlen(text), writerState);
}

void untangle::writeFormat(const char* format, ...) {
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

