#ifndef _WIN32

#include <path.hxx>

std::filesystem::path get_data_directory()
{
    static std::pair<bool, std::filesystem::path> directory{false, {}};

    if (directory.first)
        return directory.second;

    auto path = std::getenv("XDG_CONFIG_HOME");
    if (!path)
    {
        path = std::getenv("HOME"); // fallback
    }

    directory = {true, std::filesystem::path(path) / "unvm"};

    return directory.second;
}

#endif