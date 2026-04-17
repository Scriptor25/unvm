#include <unvm/util.hxx>

#include <fstream>
#include <iostream>

bool unvm::FindLocalVersion(std::filesystem::path &path)
{
    for (auto parent_path = std::filesystem::weakly_canonical(std::filesystem::current_path());;)
    {
        if (std::filesystem::exists(parent_path / ".unvm"))
        {
            path = parent_path / ".unvm";
            return true;
        }

        auto next = parent_path.parent_path();
        if (next == parent_path)
            return false;

        parent_path = next;
    }
}

int unvm::LoadLocalVersion(std::optional<std::string> &version)
{
    std::filesystem::path path;
    if (!FindLocalVersion(path))
    {
        version = std::nullopt;
        return 0;
    }

    std::ifstream stream(path);
    if (!stream)
    {
        std::cerr << "failed to open local version file '" << path.string() << "'." << std::endl;
        return 1;
    }

    std::string str;
    std::getline(stream, str);

    version = std::move(str);
    return 0;
}

int unvm::StoreLocalVersion(const std::string &version)
{
    std::ofstream stream(".unvm");
    if (!stream)
    {
        std::cerr << "failed to open local version file '.unvm'." << std::endl;
        return 1;
    }

    stream << version << std::endl;
    return 0;
}

int unvm::DeleteLocalVersion()
{
    std::filesystem::path path;
    if (!FindLocalVersion(path))
        return 0;

    if (std::error_code ec; std::filesystem::remove(path, ec), ec)
    {
        std::cerr << "failed to delete local version file '" << path.string() << "': " << ec.message() << " (" << ec.value() << ")." << std::endl;
        return 1;
    }

    return 0;
}
