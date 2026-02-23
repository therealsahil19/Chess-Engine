#include <string>

#ifdef _WIN32
#include <windows.h>

std::string GetClipboardTextFallback() {
    std::string text = "";
    if (OpenClipboard(nullptr)) {
        HANDLE hDataW = GetClipboardData(CF_UNICODETEXT);
        if (hDataW) {
            wchar_t* pwszText = static_cast<wchar_t*>(GlobalLock(hDataW));
            if (pwszText) {
                int size_needed = WideCharToMultiByte(CP_UTF8, 0, pwszText, -1, NULL, 0, NULL, NULL);
                if (size_needed > 0) {
                    std::string utf8(size_needed - 1, 0); // -1 to exclude null terminator from std::string length
                    WideCharToMultiByte(CP_UTF8, 0, pwszText, -1, &utf8[0], size_needed, NULL, NULL);
                    text = utf8;
                }
                GlobalUnlock(hDataW);
            }
        } else {
            HANDLE hDataA = GetClipboardData(CF_TEXT);
            if (hDataA) {
                char* pszText = static_cast<char*>(GlobalLock(hDataA));
                if (pszText) {
                    text = pszText;
                    GlobalUnlock(hDataA);
                }
            }
        }
        CloseClipboard();
    }
    return text;
}
#else
std::string GetClipboardTextFallback() {
    return "";
}
#endif
