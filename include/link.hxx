#pragma once

#include <filesystem>

bool CreateLink(const std::filesystem::path &link, const std::filesystem::path &target);
bool RemoveLink(const std::filesystem::path &link);
