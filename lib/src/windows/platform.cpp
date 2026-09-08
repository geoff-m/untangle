#include "platform.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace untangle {
	native_thread_handle get_current_thread() {
		return reinterpret_cast<native_thread_handle>(GetCurrentThread());
	}

	bool threads_equal(native_thread_handle x, native_thread_handle y) {
		return x == y;
	}

	std::string get_thread_name(native_thread_handle thread) {
		PWSTR threadDescr = nullptr;
		HRESULT res = GetThreadDescription(reinterpret_cast<HANDLE>(thread), &threadDescr);
		if (!SUCCEEDED(res)) {
			return "";
		}
		std::string ret(reinterpret_cast<const char*>(threadDescr));
		LocalFree(threadDescr);
		return ret;
	}

	void break_to_debugger() {
		__debugbreak();
	}

	void write_stderr(const char* text, size_t length, void*) {
		const HANDLE hStdout = GetStdHandle(STD_ERROR_HANDLE);
		if (hStdout == INVALID_HANDLE_VALUE)
			return;
		DWORD written;
		WriteFile(hStdout, text, length, &written, nullptr);
	}
}
