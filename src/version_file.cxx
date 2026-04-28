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

toolkit::result<> unvm::ReadVersionFile(std::optional<std::string> &version)
{
    std::filesystem::path path;
    if (!FindVersionFile(path))
    {
        version = std::nullopt;
        return {};
    }

    std::ifstream stream(path);
    if (!stream)
    {
        return toolkit::make_error("failed to open file '{}'.", path.string());
    }

    std::string str;
    std::getline(stream, str);

    version = std::move(str);
    return {};
}

toolkit::result<> unvm::WriteVersionFile(const std::string &version)
{
    std::ofstream stream(".unvm");
    if (!stream)
    {
        return toolkit::make_error("failed to open file '.unvm'.");
    }

    stream << version << std::endl;
    return {};
}

toolkit::result<> unvm::RemoveVersionFile()
{
    std::filesystem::path path;
    if (!FindVersionFile(path))
    {
        return {};
    }

    if (std::error_code ec; std::filesystem::remove(path, ec), ec)
    {
        return toolkit::make_error("failed to delete file '{}': {} ({}).", path.string(), ec.message(), ec.value());
    }

    return {};
}
