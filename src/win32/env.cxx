#if defined(_WIN64)

#include <util.hxx>

#include <windows.h>

int AppendUserPath(std::filesystem::path directory)
{
    directory = std::filesystem::absolute(directory);

    HKEY key;
    if (RegOpenKeyExW(
            HKEY_CURRENT_USER,
            L"Environment",
            0,
            KEY_READ | KEY_WRITE,
            &key) != ERROR_SUCCESS)
        return 1;

    DWORD type = 0;
    DWORD size = 0;
    RegQueryValueExW(key, L"Path", nullptr, &type, nullptr, &size);

    std::wstring path;
    if (size > 0)
    {
        path.resize(size / sizeof(WCHAR));
        RegQueryValueExW(
            key,
            L"Path",
            nullptr,
            &type,
            (LPBYTE) path.data(),
            &size);
        path.resize(wcslen(path.data()));
    }

    if (path.find(directory.wstring()) == std::wstring::npos)
    {
        if (!path.empty() && path.back() != L';')
        {
            path += L':';
        }

        path += directory.wstring();

        RegSetValueExW(
            key,
            L"Path",
            0,
            REG_EXPAND_SZ,
            (LPCBYTE) path.c_str(),
            (path.size() + 1) * sizeof(WCHAR));
    }

    RegCloseKey(key);

    SendMessageTimeoutW(
        HWND_BROADCAST,
        WM_SETTINGCHANGE,
        0,
        (LPARAM) L"Environment",
        SMTO_ABORTIFHUNG,
        100,
        nullptr);

    return 0;
}

#endif
