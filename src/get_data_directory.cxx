#include <unvm/util.hxx>

#if defined(SYSTEM_LINUX) || defined(SYSTEM_ANDROID) || defined(SYSTEM_DARWIN)

static std::filesystem::path get_data_directory()
{
    if (const auto xdg_config_home = std::getenv("XDG_CONFIG_HOME"))
    {
        return std::filesystem::path(xdg_config_home) / "unvm";
    }

    if (const auto home = std::getenv("HOME"))
    {
        return std::filesystem::path(home) / ".config" / "unvm";
    }

    return std::filesystem::current_path() / ".unvm";
}

#endif

#ifdef SYSTEM_WINDOWS

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

#endif

std::filesystem::path unvm::GetDataDirectory()
{
    static const auto data_directory = std::filesystem::weakly_canonical(get_data_directory());
    return data_directory;
}
