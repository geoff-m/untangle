#pragma once
#include "untangle/untangle.h"

namespace untangle {
    using WriteCallback = void (*)(const char*, size_t, void*);
    extern WriteCallback writer;
    extern void* writerState;
    void write(const char* text);
    void writeFormat(const char* format, ...);
}