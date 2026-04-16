#ifdef SYSTEM_WINDOWS

#include <unvm/util.hxx>

#include <shlobj.h>
#include <windows.h>

static std::filesystem::path get_data_directory()
{
    PWSTR path = nullptr;
    SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &path);

    std::filesystem::path directory(path);

    CoTaskMemFree(path);

    return directory / "unvm";
}

std::filesystem::path unvm::GetDataDirectory()
{
    static const auto data_directory = std::filesystem::weakly_canonical(get_data_directory());
    return data_directory;
}

#endif
