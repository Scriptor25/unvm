#ifdef SYSTEM_DARWIN

#include <util.hxx>

std::filesystem::path GetDataDirectory()
{
    static std::pair<bool, std::filesystem::path> directory{ false, {} };

    if (directory.first)
    {
        return directory.second;
    }

    std::filesystem::path path;
    if (const auto home = std::getenv("HOME"))
    {
        path = std::filesystem::path(home) / "Library" / "Application Support" / "unvm";
    }
    else
    {
        path = std::filesystem::current_path() / ".unvm";
    }

    directory = { true, path };

    return directory.second;
}

#endif
