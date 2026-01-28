#ifdef SYSTEM_DARWIN

#include <util.hxx>

#include <unistd.h>

int CreateLink(const std::filesystem::path &link, const std::filesystem::path &target)
{
    if (!std::filesystem::exists(target) || !std::filesystem::is_directory(target))
    {
        return 1;
    }

    if (std::filesystem::exists(link) || std::filesystem::is_symlink(link))
    {
        return 1;
    }

    const auto link_string = link.string();
    const auto target_string = target.string();

    if (const auto error = symlink(target_string.c_str(), link_string.c_str()))
    {
        return error;
    }

    return 0;
}

int RemoveLink(const std::filesystem::path &link)
{
    if (!std::filesystem::exists(link) || !std::filesystem::is_symlink(link))
    {
        return 1;
    }

    const auto link_string = link.string();

    if (const auto error = unlink(link_string.c_str()))
    {
        return error;
    }

    return 0;
}

#endif
