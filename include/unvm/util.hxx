#pragma once

#include <filesystem>
#include <map>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace unvm
{
    std::filesystem::path GetDataDirectory();

    std::istream &GetLine(std::istream &stream, std::string &string, std::string_view delim);

    std::string Trim(std::string string);
    std::string Lower(std::string string);

    std::vector<std::string> Split(const std::string &str, char delim);
    std::string Join(const std::vector<std::string> &vec, char delim);

    bool FindVersionFile(std::filesystem::path &path);

    int ReadVersionFile(std::optional<std::string> &version);
    int WriteVersionFile(const std::string &version);
    int RemoveVersionFile();
}

template<typename K, typename V>
std::ostream &operator<<(std::ostream &stream, const std::map<K, V> &map)
{
    stream << "{ ";
    for (auto it = map.begin(); it != map.end(); ++it)
    {
        if (it != map.begin())
        {
            stream << ", ";
        }
        stream << it->first << ": " << it->second;
    }
    return stream << " }";
}
