#include "platform.h"
#include "Windows.h"
#include <threads.h>

namespace untangle {
	using _Mtx_t = void*;
	using _Mtx_init_t = int(__cdecl*)(_Mtx_t*, int);
	using _Mtx_destroy_t = void(__cdecl*)(_Mtx_t);
	using _Mtx_lock_t = int(__cdecl*)(_Mtx_t);
	using _Mtx_unlock_t = int(__cdecl*)(_Mtx_t);

	static _Mtx_t deadlockCheckMutexValue;
	native_mutex_handle deadlockCheckMutex = reinterpret_cast<native_mutex_handle>(&deadlockCheckMutexValue);

	static native_mutex_handle mutexInfosMutexValue;
	native_mutex_handle mutexInfosMutex = reinterpret_cast<native_mutex_handle>(&mutexInfosMutexValue);

	OriginalFunctions originalFunctions;
	static _Mtx_init_t orig_mtxInit;
	int OriginalFunctions::mutex_init(native_mutex_handle mutex, const void* options) {
		return orig_mtxInit(reinterpret_cast<_Mtx_t*>(&mutex), static_cast<int>(reinterpret_cast<uint64_t>(options)));
	}

	static _Mtx_destroy_t orig_mtxDestroy;
	void OriginalFunctions::mutex_destroy(native_mutex_handle mutex) {
		return orig_mtxDestroy(reinterpret_cast<_Mtx_t>(mutex));
	}

	static _Mtx_lock_t orig_mtxLock;
	int OriginalFunctions::mutex_lock(native_mutex_handle mutex) {
		return orig_mtxLock(reinterpret_cast<_Mtx_t>(mutex));
	}

	static _Mtx_unlock_t orig_mtxUnlock;
	int OriginalFunctions::mutex_unlock(native_mutex_handle mutex) {
		return orig_mtxUnlock(reinterpret_cast<_Mtx_t>(mutex));
	}

	int OriginalFunctions::thread_join(native_thread_handle thread, void** thread_return_value) {
		// todo
		return 0;
	}

	void OriginalFunctions::initialize() {
		HMODULE vcRuntime = GetModuleHandleW(L"vcruntime140.dll");
		if (!vcRuntime) {
			fprintf(stderr, "untangle: Setup failed: Failed to find VC runtime\n");
			abort();
		}
		orig_mtxInit = reinterpret_cast<_Mtx_init_t>(GetProcAddress(vcRuntime, "_Mtx_init"));
		orig_mtxDestroy = reinterpret_cast<_Mtx_destroy_t>(GetProcAddress(vcRuntime, "_Mtx_destroy"));
		orig_mtxLock = reinterpret_cast<_Mtx_lock_t>(GetProcAddress(vcRuntime, "_Mtx_lock"));
		orig_mtxUnlock = reinterpret_cast<_Mtx_unlock_t>(GetProcAddress(vcRuntime, "_Mtx_unlock"));

		if (!(orig_mtxInit && orig_mtxDestroy && orig_mtxLock && orig_mtxUnlock)) {
			fprintf(stderr, "untangle: Setup failed: Failed to find functions for hooking");
			abort();
		}
	}
}
