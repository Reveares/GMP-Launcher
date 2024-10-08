#pragma once
#include "win32Utils.h"
#include <string>

inline void startZSpy(const std::wstring &gothicPath) {
    const std::wstring zSpyApp(L"zSpy.exe");
    if (isProcessRunning(zSpyApp)) {
        return;
    }

    const std::wstring zSpyWorkdir(gothicPath + L"\\_work\\tools\\zSpy");
    const std::wstring zSpyPath(zSpyWorkdir + L"\\" + zSpyApp);
    if (!fileExists(zSpyPath)) {
        fprintf(stderr, "zSpy EXE not found in path \"%ls\"\n", zSpyPath.c_str());
        return;
    }

    PROCESS_INFORMATION pi{};
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    //si.dwFlags = STARTF_USESHOWWINDOW;
    //si.wShowWindow = SW_MINIMIZE;
    if (CreateProcessW(zSpyPath.c_str(), nullptr, nullptr, nullptr, FALSE, 0, nullptr, zSpyWorkdir.c_str(), &si, &pi)) {
        if (WaitForInputIdle(pi.hProcess, INFINITE) == WAIT_TIMEOUT) {
            fprintf(stderr, "WaitForSingleObject time out in zSpy process. GetLastError: %lu\n", GetLastError());
            return;
        }
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}
