#pragma once
#include "Windows.h"
#include <string>

inline bool isSlash(const wchar_t c) {
    return c == L'\\' || c == L'/';
}

inline std::wstring parentPath(const std::wstring &path) {
    const wchar_t *const first = path.data();
    const wchar_t *last = first + path.size();
    // Remove everything until the first slash
    while (first != last && !isSlash(last[-1])) {
        --last;
    }
    // Now remove the slashes
    while (first != last && isSlash(last[-1])) {
        --last;
    }

    return {first, last};
}

inline std::wstring absolutePath(const std::wstring &path) {
    const DWORD size = GetFullPathNameW(path.c_str(), 0, nullptr, nullptr);
    std::wstring absolutePath;
    absolutePath.resize(size);
    GetFullPathNameW(path.c_str(), size, absolutePath.data(), nullptr);
    return absolutePath;
}

inline bool fileExists(const std::wstring &path) {
    DWORD result = GetFileAttributesW(path.c_str());
    return result != INVALID_FILE_ATTRIBUTES && !(result & FILE_ATTRIBUTE_DIRECTORY);
}
