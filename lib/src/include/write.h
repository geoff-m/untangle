#pragma once
#include "untangle/untangle.h"

namespace untangle::writer {
    using WriteCallback = void (*)(const char*, size_t, void*);
    extern WriteCallback writeCb;
    extern void* writerState;
    void write(const char* text);
    void writeFormat(const char* format, ...);

    void constructWrite();
    void destroyWrite();
}