#include "OutputCapture.h"

void OutputCapture::append(const char* text, size_t length) {
    ss.write(text, static_cast<long>(length));
}

const std::stringstream& OutputCapture::getOutput() {
    return ss;
}

void capture(const char* text, size_t length, void* outputCapture) {
    static_cast<OutputCapture*>(outputCapture)->append(text, length);
}