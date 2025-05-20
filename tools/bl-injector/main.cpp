//

#include <cstdio>
#include <cwchar>
#include <string>
#include <system_error>

#include "binlab/Config.h"

#ifdef _WIN32
#include <Windows.h>
#include <memoryapi.h>
#include <tlhelp32.h>
#include <winuser.h>

int Inject1(const std::size_t pid, const wchar_t* dll) {
  int errc = 0;

  HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, pid);
  if (hProcess) {
    LPVOID pDllPath = ::VirtualAllocEx(hProcess, 0, std::wcslen(dll) * sizeof(dll[0]), MEM_COMMIT, PAGE_READWRITE);
    if (pDllPath) {
      if (::WriteProcessMemory(hProcess, pDllPath, (LPVOID)dll, std::wcslen(dll) * sizeof(dll[0]), NULL)) {
        HANDLE hThread = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)&LoadLibraryW, pDllPath, 0, NULL);
        if (hThread) {
          WaitForSingleObject(hThread, INFINITE);
          CloseHandle(hThread);
        } else {
          errc = ::GetLastError();
        }
      } else {
        errc = ::GetLastError();
      }
      ::VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
    } else {
      errc = ::GetLastError();
    }
    ::CloseHandle(hProcess);
  } else {
    errc = ::GetLastError();
  }
  return errc;
}

int Inject4(const std::size_t pid, const wchar_t* dll) {
  int errc = 0;

  HMODULE hModDll = LoadLibraryW(dll);

  HOOKPROC procAddress = (HOOKPROC)GetProcAddress(hModDll, "HookProcedure");

  HANDLE hThreadSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
  if (hThreadSnap != INVALID_HANDLE_VALUE) {
    THREADENTRY32 te32{.dwSize = sizeof(THREADENTRY32)};
    DWORD threadId = 0;
    if (::Thread32First(hThreadSnap, &te32)) {
      for (;; Thread32Next(hThreadSnap, &te32)) {
        if (te32.th32OwnerProcessID == pid) {
          threadId = te32.th32ThreadID;
          HANDLE hThread = OpenThread(READ_CONTROL, FALSE, te32.th32ThreadID);
          if (hThread) {
            HHOOK hookHandle = SetWindowsHookExW(WH_KEYBOARD, procAddress, hModDll, (DWORD)threadId);
            if (hookHandle) {
              system("PAUSE");
              UnhookWindowsHookEx(hookHandle);
            } else {
              errc = ::GetLastError();
            }
            break;
          }
        }
      }
      if (!threadId) {
        std::wprintf(L"No Thread for use in process\n");
      }
    } else {
      errc = ::GetLastError();
    }
  } else {
    errc = ::GetLastError();
  }
  return errc;
}

int wmain(int argc, wchar_t* argv[]) {
  if (argc < 2) {
    std::wprintf(L"injector ver: %d.%d\n", BINLAB_VERSION_MAJOR, BINLAB_VERSION_MINOR);
    std::wprintf(L"\ninjector [pid] [dll path]\n");
    return 0;
  }

  auto pid = std::stoull(argv[1]);
  Inject4(pid, argv[2]);
  return 0;
}
#endif  // !_WIN32
