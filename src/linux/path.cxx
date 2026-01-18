#if defined(__x86_64__) || defined(__amd64__)

#include <util.hxx>

std::filesystem::path GetDataDirectory()
{
    static std::pair<bool, std::filesystem::path> directory{ false, {} };

    if (directory.first)
        return directory.second;

    std::filesystem::path path;
    if (const auto xdg_config_home = std::getenv("XDG_CONFIG_HOME"))
    {
        path = std::filesystem::path(xdg_config_home) / "unvm";
    }
    else if (const auto home = std::getenv("HOME"))
    {
        path = std::filesystem::path(home) / ".config" / "unvm";
    }
    else
    {
        path = std::filesystem::current_path() / ".unvm";
    }

    directory = { true, path };

    return directory.second;
}

#endif
