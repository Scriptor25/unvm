#pragma once

#include <unvm/config.hxx>
#include <unvm/version.hxx>
#include <unvm/http/http.hxx>

#include <filesystem>
#include <istream>
#include <string_view>

namespace unvm
{
    void PrintManual();

    int LoadVersionTable(http::HttpClient &client, VersionTable &table, bool online);

    const VersionEntry *FindEffectiveVersion(const VersionTable &table, std::string_view version);

    int UnpackArchive(std::istream &stream, const std::filesystem::path &directory);

    int Install(Config &config, http::HttpClient &client, std::string_view version, const VersionEntry &entry);
    int Install(Config &config, http::HttpClient &client, std::string_view version);

    int Remove(Config &config, std::string_view version, const VersionEntry &entry);
    int Remove(Config &config, http::HttpClient &client, std::string_view version);
    
    int Use(Config &config, const std::string_view &version, const VersionEntry &entry);
    int Use(Config &config, http::HttpClient &client, std::string_view version);

    int List(Config &config, http::HttpClient &client, const bool available);

    int Workspace(Config &config, http::HttpClient &client);
}
