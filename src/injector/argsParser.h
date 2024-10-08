#pragma once
#include <string>

struct ProgramArgs {
    std::wstring gothic{L"Gothic2.exe"};
    std::wstring dll{L"gmp\\gmp.dll"};
    std::wstring host{L"localhost:28960"};
    std::wstring nickname;
    std::wstring debugLevel{L"-1"};
    bool enableGothicException{false};
};

inline ProgramArgs parseProgramArgs(int argc, wchar_t *argv[]) {
    ProgramArgs programArgs{};

    constexpr wchar_t gothicArg[] = L"--gothic=";
    constexpr wchar_t dllArg[] = L"--dll=";
    constexpr wchar_t hostArg[] = L"--host=";
    constexpr wchar_t nicknameArg[] = L"--nickname=";
    constexpr wchar_t debugLevelArg[] = L"--debug=";
    constexpr wchar_t exceptionArg[] = L"--exception";
    constexpr wchar_t helpArg[] = L"--help";
    for (int i = 1; i < argc; i++) {
        std::wstring_view arg(argv[i]);
        if (arg.find(gothicArg) != std::wstring_view::npos) {
            programArgs.gothic = arg.substr((sizeof(gothicArg) - 1) / sizeof(wchar_t));
        } else if (arg.find(dllArg) != std::wstring_view::npos) {
            programArgs.dll = arg.substr((sizeof(dllArg) - 1) / sizeof(wchar_t));
        } else if (arg.find(hostArg) != std::wstring_view::npos) {
            programArgs.host = arg.substr((sizeof(hostArg) - 1) / sizeof(wchar_t));
        } else if (arg.find(nicknameArg) != std::wstring_view::npos) {
            programArgs.nickname = arg.substr((sizeof(nicknameArg) - 1) / sizeof(wchar_t));
        } else if (arg.find(debugLevelArg) != std::wstring_view::npos) {
            programArgs.debugLevel = arg.substr((sizeof(debugLevelArg) - 1) / sizeof(wchar_t));
        } else if (arg.find(exceptionArg) != std::wstring_view::npos) {
            programArgs.enableGothicException = true;
        } else if (arg.find(helpArg) != std::wstring_view::npos) {
            printf(
                "usage: gmpinjector [%ls] [%ls<path>] [%ls<path>] [%ls<host:port>] [%ls<name>] [%ls<number>] [%ls]\n",
                helpArg, gothicArg, dllArg, hostArg, nicknameArg, debugLevelArg, exceptionArg);
            std::exit(EXIT_SUCCESS);
        }
    }
    return programArgs;
}
