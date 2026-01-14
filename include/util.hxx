#pragma once

#include <filesystem>

bool AppendUserPath(std::filesystem::path directory);

bool CreateLink(const std::filesystem::path &link, const std::filesystem::path &target);
bool RemoveLink(const std::filesystem::path &link);

std::filesystem::path GetDataDirectory();
