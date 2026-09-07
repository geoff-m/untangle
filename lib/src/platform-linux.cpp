#include "platform.h"
#include <pthread.h>
#include <unistd.h>

using namespace untangle;

native_thread_handle get_current_thread() {
	return pthread_self();
}

bool threads_equal(native_thread_handle x, native_thread_handle y) {
    return pthread_equal(x, y);
}

std::string get_thread_name(native_thread_handle thread) {
    constexpr auto MAX_NAME_LENGTH = 16;
    char name[MAX_NAME_LENGTH] = {};
    pthread_getname_np(thread, name, MAX_NAME_LENGTH);
    const auto length = strlen(name);
    std::string ret;
    ret.reserve(length + 2);
    ret.append('"');
    ret.append(name);
    ret.append('"');
    return ret;
    writeFormat("\"%s\" (%#lx)", name, thread);
}

void break_to_debugger() {
    raise(SIGTRAP);
}

void write_stdout(const char* text, size_t length, void*) {
    ::write(STDERR_FILENO, text, length);
}