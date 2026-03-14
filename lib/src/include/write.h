#pragma once
#include "untangle/untangle.h"

namespace untangle {
    extern WriteCallback writer;
    extern void* writerState;
    void write(const char* text);
    void writeFormat(const char* format, ...);
}