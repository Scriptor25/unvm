#pragma once

#include <filesystem>

int AppendUserPath(std::filesystem::path directory);

int CreateLink(const std::filesystem::path &link, const std::filesystem::path &target);
int RemoveLink(const std::filesystem::path &link);

std::filesystem::path GetDataDirectory();

std::istream &GetLine(std::istream &stream, std::string &string, const std::string_view &delim);

std::string Trim(std::string string);
std::string Lower(std::string string);
