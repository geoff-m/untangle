#pragma once
#include <sstream>

class OutputCapture {
    std::stringstream ss;

public:
    void append(const char* text, size_t length);
    [[nodiscard]] const std::stringstream& getOutput();
};

void capture(const char* text, size_t length, void* outputCapture);