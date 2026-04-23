#include <unvm/util.hxx>

#include <fstream>
#include <iostream>

bool unvm::FindVersionFile(std::filesystem::path &path)
{
    for (auto parent_path = std::filesystem::weakly_canonical(std::filesystem::current_path());;)
    {
        if (auto entry = parent_path / ".unvm"; exists(entry))
        {
            path = std::move(entry);
            return true;
        }

        auto next = parent_path.parent_path();
        if (next == parent_path)
        {
            return false;
        }

        parent_path = next;
    }
}

int unvm::ReadVersionFile(std::optional<std::string> &version)
{
    std::filesystem::path path;
    if (!FindVersionFile(path))
    {
        version = std::nullopt;
        return 0;
    }

    std::ifstream stream(path);
    if (!stream)
    {
        std::cerr << "failed to open file '" << path.string() << "'." << std::endl;
        return 1;
    }

    std::string str;
    std::getline(stream, str);

    version = std::move(str);
    return 0;
}

int unvm::WriteVersionFile(const std::string &version)
{
    std::ofstream stream(".unvm");
    if (!stream)
    {
        std::cerr << "failed to open file '.unvm'." << std::endl;
        return 1;
    }

    stream << version << std::endl;
    return 0;
}

int unvm::RemoveVersionFile()
{
    std::filesystem::path path;
    if (!FindVersionFile(path))
    {
        return 0;
    }

    if (std::error_code ec; std::filesystem::remove(path, ec), ec)
    {
        std::cerr
                << "failed to delete file '"
                << path.string()
                << "': "
                << ec.message()
                << " ("
                << ec.value()
                << ")."
                << std::endl;
        return 1;
    }

    return 0;
}
