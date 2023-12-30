#include "Windows.h"
#include "pathUtils.h"
#include "argsParser.h"
#include <string>

int handleError(const PROCESS_INFORMATION& pi, const char format[])
{
    fprintf(stderr, format, GetLastError());
    TerminateProcess(pi.hProcess, EXIT_FAILURE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return EXIT_FAILURE;
}

int inject(const std::wstring& gothicExePath, const std::wstring& dllPath, const std::wstring& serverHost,
           const std::wstring& nickname)
{
    if (!fileExists(gothicExePath)) {
        fprintf(stderr, "Gothic EXE not found in path \"%ls\"\n", gothicExePath.c_str());
        return EXIT_FAILURE;
    }

    const std::wstring dllAbsolutePath = absolutePath(dllPath);
    if (!fileExists(dllAbsolutePath)) {
        fprintf(stderr, "GMP DLL not found in path \"%ls\"\n", dllAbsolutePath.c_str());
        return EXIT_FAILURE;
    }

    /**
     * ZNOEXHND      => Disables Gothic exception handling
     * --gmphost     => Passes the GMP server address to Gothic
     * --gmpnickname => Passes the GMP nickname to Gothic
     */
    std::wstring args = L"\"" + gothicExePath + L"\" ZNOEXHND \"--gmphost=" + serverHost + L"\" \"--gmpnickname=" +
        nickname + L"\"";

    PROCESS_INFORMATION pi{};
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    if (!CreateProcessW(gothicExePath.c_str(), args.data(), nullptr, nullptr, FALSE, CREATE_SUSPENDED, nullptr,
                        parentPath(gothicExePath).c_str(), &si, &pi))
    {
        fprintf(stderr, "Couldn't CreateProcessW a Gothic process. GetLastError: %lu\n", GetLastError());
        return EXIT_FAILURE;
    }

    const size_t dllPathSize{(dllAbsolutePath.size() + 1) * sizeof(wchar_t)};
    LPVOID buffer = VirtualAllocEx(pi.hProcess, nullptr, dllPathSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!buffer)
    {
        return handleError(pi, "Couldn't VirtualAllocEx in Gothic process. GetLastError: %lu\n");
    }

    if (!WriteProcessMemory(pi.hProcess, buffer, dllAbsolutePath.c_str(), dllPathSize, nullptr))
    {
        return handleError(pi, "Couldn't WriteProcessMemory in Gothic process. GetLastError: %lu\n");
    }

    FARPROC func = GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");
    if (!func)
    {
        return handleError(pi, "Couldn't GetProcAddress from kernel32.dll for function LoadLibraryW. GetLastError: %lu\n");
    }

    HANDLE remoteThread = CreateRemoteThread(pi.hProcess, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(func),
                                             buffer, 0, nullptr);
    if (!remoteThread)
    {
        return handleError(pi, "Couldn't CreateRemoteThread in Gothic process. GetLastError: %lu\n");
    }

    if (WaitForSingleObject(remoteThread, INFINITE) == WAIT_TIMEOUT)
    {
        const int result = handleError(pi, "WaitForSingleObject time out in Gothic process. GetLastError: %lu\n");
        CloseHandle(remoteThread);
        return result;
    }

    CloseHandle(remoteThread);
    VirtualFreeEx(pi.hProcess, buffer, dllPathSize, MEM_RELEASE);

    ResumeThread(pi.hThread);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return EXIT_SUCCESS;
}

int wmain(int argc, wchar_t* argv[])
{
    const auto [gothic, dll, host, nickname] = parseProgramArgs(argc, argv);
    return inject(gothic, dll, host, nickname);
}
