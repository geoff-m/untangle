#include "platform.h"
#include "write.h"
#include "windows/definitions.h"
#include "detours/detours.h"

using namespace untangle;

void ctor() {
    writer::constructWrite();
    originalFunctions.initialize();
    constexpr int NONREENTRANT_MUTEX_OPTION = 0;
    originalFunctions.mutex_init(mutexInfosMutex, reinterpret_cast<const void*>(NONREENTRANT_MUTEX_OPTION));
    originalFunctions.mutex_init(deadlockCheckMutex, reinterpret_cast<const void*>(NONREENTRANT_MUTEX_OPTION));
}

void dtor() {
    writer::destroyWrite();
}

static PVOID dllNotificationCallbackCookie;
bool unregisterDllNotificationCallback() {
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (!hNtdll) {
        return false;
    }
    const auto pLdrUnregisterDllNotification = (_LdrUnregisterDllNotification)GetProcAddress(hNtdll, "LdrUnregisterDllNotification");
    if (0 != pLdrUnregisterDllNotification(dllNotificationCallbackCookie)) {
        return false;
    }
    return true;
}

static VOID onDllLoad(ULONG NotificationReason, const PLDR_DLL_NOTIFICATION_DATA notificationData, PVOID context) {
    auto name = notificationData->Loaded.BaseDllName;
    fprintf(stderr, "untangle: dll loaded: %Z\n", name);
    if (0 == CompareStringEx(NULL, LINGUISTIC_IGNORECASE, L"ntdll.dll", -1, name->pBuffer, name->Length, 0, NULL, 0)) {
        ctor();
        if (!unregisterDllNotificationCallback()) {
            fprintf(stderr, "untangle: Failed to unregister DLL notification callback\n");
        }
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID /*reserved*/)
{
    switch (reason) {
    case DLL_PROCESS_ATTACH: {
        DetourRestoreAfterWith();
        HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
        if (!hNtdll) {
            fprintf(stderr, "untangle: Setup failed: Cannot get handle to ntdll\n");
            return FALSE;
        }
        auto pLdrRegisterDllNotification = (_LdrRegisterDllNotification)GetProcAddress(hNtdll, "LdrRegisterDllNotification");
        NTSTATUS status = pLdrRegisterDllNotification(
            0,
            (PLDR_DLL_NOTIFICATION_FUNCTION)onDllLoad,
            NULL,
            &dllNotificationCallbackCookie);
        if (status != 0) {
            fprintf(stderr, "untangle: Setup failed: Cannot register DLL load notification callback\n");
            return FALSE;
        }
        break;
    }
    case DLL_PROCESS_DETACH:
        dtor();
        break;
    }
    return TRUE;
}