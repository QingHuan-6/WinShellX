#include "utils/WinApiError.h"

#include "utils/StringUtils.h"

#include <windows.h>

std::string getLastErrorMessage() {
    DWORD errorCode = GetLastError();
    if (errorCode == 0) {
        return "No error";
    }

    LPSTR buffer = nullptr;
    DWORD size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPSTR>(&buffer),
        0,
        nullptr);

    std::string message = size && buffer ? buffer : "Unknown Windows API error";
    if (buffer) {
        LocalFree(buffer);
    }

    return trim(message);
}
