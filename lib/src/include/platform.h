#pragma once
#include "untangle/untangle.h"
#include <string>

namespace untangle {
	[[nodiscard]] native_thread_handle get_current_thread();
	[[nodiscard]] bool threads_equal(native_thread_handle x, native_thread_handle y);
	[[nodiscard]] std::string get_thread_name(native_thread_handle thread);
	void break_to_debugger();
	void unreachable();

	struct OriginalFunctions {
		void initialize();
		int mutex_lock(native_mutex_handle mutex);
		int mutex_unlock(native_mutex_handle mutex);
	};
	extern OriginalFunctions originalFunctions;
	extern native_mutex_handle deadlockCheckMutex;

	void write_stdout(const char* text, size_t length, void*);
}