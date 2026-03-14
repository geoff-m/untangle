#pragma once
#include <sstream>

class OutputCapture {
    std::stringstream ss;

public:
    void append(const char* text, size_t length);
    [[nodiscard]] std::istream& getOutput();
};

void capture(const char* text, size_t length, void* outputCapture);