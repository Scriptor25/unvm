#if defined(SYSTEM_LINUX) || defined(SYSTEM_DARWIN)

#include <util.hxx>

#include <iostream>

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

    std::error_code error;
    std::filesystem::create_directory_symlink(target, link, error);

    if (error)
    {
        std::cerr << "failed to create link: " << error.message() << std::endl;
        return error.value();
    }

    return 0;
}

int RemoveLink(const std::filesystem::path &link)
{
    if (!std::filesystem::exists(link) || !std::filesystem::is_symlink(link))
    {
        return 1;
    }

    std::error_code error;
    std::filesystem::remove(link, error);

    if (error)
    {
        std::cerr << "failed to remove link: " << error.message() << std::endl;
        return error.value();
    }

    return 0;
}

#endif
