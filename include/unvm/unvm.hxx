#pragma once

#include <unvm/config.hxx>
#include <unvm/version.hxx>
#include <unvm/http/http.hxx>

#include <toolkit/result.hxx>

#include <filesystem>
#include <string_view>

namespace unvm
{
    void PrintManual();

    toolkit::result<> LoadVersionTable(http::HttpClient &client, VersionTable &table, bool online);

    const VersionEntry *FindEffectiveVersion(const VersionTable &table, std::string_view version);

    toolkit::result<> UnpackArchive(std::istream &stream, const std::filesystem::path &directory);

    toolkit::result<> Install(Config &config, http::HttpClient &client, std::string_view version, const VersionEntry &entry);
    toolkit::result<> Install(Config &config, http::HttpClient &client, std::string_view version);

    toolkit::result<> Remove(Config &config, http::HttpClient &client, std::string_view version);

    toolkit::result<> Use(Config &config, http::HttpClient &client, std::string_view version, bool local);

    toolkit::result<> List(const Config &config, http::HttpClient &client, bool available);
}
