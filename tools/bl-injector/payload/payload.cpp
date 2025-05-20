//

#include <Windows.h>

#include <cwchar>

extern "C" __declspec(dllexport) void HookProcedure() {
  DWORD pid = ::GetCurrentProcessId();
  WCHAR path[MAX_PATH] = {0};
  wchar_t buffer[0x1000] = {0};
  if (::GetModuleFileNameW(NULL, path, MAX_PATH) && swprintf(buffer, L"PID: %6d\npath: %s", pid, path) > 0) {
    ::MessageBoxW(0, buffer, L"HookProcedure", MB_ICONINFORMATION);
  }
}

DWORD WINAPI ThreadShow(LPVOID lpParameter) {
  if (lpParameter) {
    LPCWSTR reason = nullptr;
    switch (*static_cast<DWORD*>(lpParameter)) {
      case DLL_PROCESS_ATTACH:
        reason = L"DLL_PROCESS_ATTACH";
        break;
      case DLL_THREAD_ATTACH:
        reason = L"DLL_THREAD_ATTACH";
        break;
      case DLL_THREAD_DETACH:
        reason = L"DLL_THREAD_DETACH";
        break;
      case DLL_PROCESS_DETACH:
        reason = L"DLL_PROCESS_DETACH";
        break;
    }

    DWORD pid = ::GetCurrentProcessId();
    WCHAR path[MAX_PATH] = {0};
    wchar_t buffer[0x1000] = {0};
    if (::GetModuleFileNameW(NULL, path, MAX_PATH) && swprintf(buffer, L"PID(%s): %6d\npath: %s", reason, pid, path) > 0) {
      ::MessageBoxW(0, buffer, L"DLL Inject", MB_ICONINFORMATION);
    }
  }
  return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
  ThreadShow(static_cast<LPVOID>(&ul_reason_for_call));
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
      break;
    case DLL_THREAD_ATTACH:
      break;
    case DLL_THREAD_DETACH:
      break;
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}
