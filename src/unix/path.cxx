#if defined(SYSTEM_LINUX) || defined(SYSTEM_ANDROID) || defined(SYSTEM_DARWIN)

#include <unvm/util.hxx>

static std::filesystem::path get_data_directory()
{
    if (const auto xdg_config_home = std::getenv("XDG_CONFIG_HOME"))
        return std::filesystem::path(xdg_config_home) / "unvm";
    else if (const auto home = std::getenv("HOME"))
        return std::filesystem::path(home) / ".config" / "unvm";
    else
        return std::filesystem::current_path() / ".unvm";
}

std::filesystem::path unvm::GetDataDirectory()
{
    static const auto data_directory = std::filesystem::weakly_canonical(get_data_directory());
    return data_directory;
}

#endif
