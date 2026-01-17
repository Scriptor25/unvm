#ifdef _WIN32

#include <util.hxx>

#include <shlobj.h>
#include <windows.h>

std::filesystem::path GetDataDirectory()
{
    static std::pair<bool, std::filesystem::path> directory{ false, {} };

    if (directory.first)
        return directory.second;

    PWSTR path = nullptr;
    SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &path);

    directory = { true, std::filesystem::path(path) / "unvm" };

    CoTaskMemFree(path);

    return directory.second;
}

#endif
