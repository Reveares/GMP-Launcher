#include "Windows.h"
#include "tlhelp32.h"
#include "win32Utils.h"
#include "argsParser.h"
#include "zSpy.h"
#include <string>

int inject(const ProgramArgs &programArgs) {
    if (!fileExists(programArgs.gothic)) {
        fprintf(stderr, "Gothic EXE not found in path \"%ls\"\n", programArgs.gothic.c_str());
        return EXIT_FAILURE;
    }

    const std::wstring dllAbsolutePath = absolutePath(programArgs.dll);
    if (!fileExists(dllAbsolutePath)) {
        fprintf(stderr, "GMP DLL not found in path \"%ls\"\n", dllAbsolutePath.c_str());
        return EXIT_FAILURE;
    }

    /**
     * ZNOEXHND      => Gothic: Disables exception handling
     * -zlog:9,s     => Gothic: Enables logging to zspy. The number changes the log level
     * --gmphost     => GMP: server address
     * --gmpnickname => GMP: user nickname
     */
    std::wstring args = L"\"" + programArgs.gothic + L"\" \"--gmphost=" + programArgs.host + L"\" \"--gmpnickname=" +
                        programArgs.nickname + L"\"";
    if (!programArgs.enableGothicException) {
        args += L" ZNOEXHND";
    }
    if (programArgs.debugLevel != L"-1") {
        args += L" -zlog:" + programArgs.debugLevel + L",s";
        // Remove 'System/Gothic2.exe' from path
        startZSpy(parentPath(parentPath(programArgs.gothic)));
    }

    PROCESS_INFORMATION pi{};
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    if (CreateProcessW(programArgs.gothic.c_str(), args.data(), nullptr, nullptr, FALSE, CREATE_SUSPENDED, nullptr,
                       parentPath(programArgs.gothic).c_str(), &si, &pi) == FALSE) {
        fprintf(stderr, "Couldn't CreateProcessW a Gothic process. GetLastError: %lu\n", GetLastError());
        return EXIT_FAILURE;
    }

    auto handleError = [&pi](const char format[]) {
        fprintf(stderr, format, GetLastError());
        TerminateProcess(pi.hProcess, EXIT_FAILURE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return EXIT_FAILURE;
    };

    const size_t dllPathSize{(dllAbsolutePath.size() + 1) * sizeof(wchar_t)};
    LPVOID buffer = VirtualAllocEx(pi.hProcess, nullptr, dllPathSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!buffer) {
        return handleError("Couldn't VirtualAllocEx in Gothic process. GetLastError: %lu\n");
    }

    if (WriteProcessMemory(pi.hProcess, buffer, dllAbsolutePath.c_str(), dllPathSize, nullptr) == FALSE) {
        return handleError("Couldn't WriteProcessMemory in Gothic process. GetLastError: %lu\n");
    }

    FARPROC func = GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");
    if (!func) {
        return handleError("Couldn't GetProcAddress from kernel32.dll for function LoadLibraryW. GetLastError: %lu\n");
    }

    HANDLE remoteThread = CreateRemoteThread(pi.hProcess, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(func),
                                             buffer, 0, nullptr);
    if (!remoteThread) {
        return handleError("Couldn't CreateRemoteThread in Gothic process. GetLastError: %lu\n");
    }

    const DWORD result = WaitForSingleObject(remoteThread, INFINITE);
    CloseHandle(remoteThread);
    VirtualFreeEx(pi.hProcess, buffer, dllPathSize, MEM_RELEASE);
    if (result == WAIT_TIMEOUT) {
        return handleError("WaitForSingleObject time out in Gothic process. GetLastError: %lu\n");
    }

    ResumeThread(pi.hThread);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return EXIT_SUCCESS;
}

int wmain(int argc, wchar_t *argv[]) {
    const auto programArgs = parseProgramArgs(argc, argv);
    return inject(programArgs);
}
