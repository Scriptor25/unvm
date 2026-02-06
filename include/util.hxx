#pragma once

#include <filesystem>
#include <map>

int AppendUserPath(std::filesystem::path directory);

int CreateLink(const std::filesystem::path &link, const std::filesystem::path &target);
int RemoveLink(const std::filesystem::path &link);

std::filesystem::path GetDataDirectory();

std::istream &GetLine(std::istream &stream, std::string &string, const std::string_view &delim);

std::string Trim(std::string string);
std::string Lower(std::string string);

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
