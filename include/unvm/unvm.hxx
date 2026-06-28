#pragma once

#include <unvm/config.hxx>
#include <unvm/version.hxx>
#include <unvm/http/http.hxx>

#include <toolkit/args.hxx>
#include <toolkit/result.hxx>

#include <filesystem>
#include <string_view>

namespace unvm
{
    void PrintManual();

    [[nodiscard]] toolkit::result<> LoadVersionTable(
        http::HttpClient &client,
        VersionTable &table,
        bool online);

    void FilterVersionTable(const Config &config, VersionTable &table, bool supported = false, bool installed = false);

    const VersionEntry *FindEffectiveVersion(
        const VersionTable &table,
        std::string_view version,
        bool &matched);

    toolkit::result<const VersionEntry *> FindVersionEntry(const VersionTable &table, std::string_view version);

    [[nodiscard]] toolkit::result<> UnpackArchive(
        std::istream &stream,
        const std::filesystem::path &directory);

    [[nodiscard]] toolkit::result<> Install(
        Config &config,
        http::HttpClient &client,
        std::string_view version,
        const VersionEntry &entry);
    [[nodiscard]] toolkit::result<> Install(
        Config &config,
        http::HttpClient &client,
        std::string_view version);

    [[nodiscard]] toolkit::result<> Track(
        Config &config,
        http::HttpClient &client,
        std::string_view name,
        std::string_view version);
    [[nodiscard]] toolkit::result<> Untrack(
        Config &config,
        http::HttpClient &client,
        std::string_view name,
        bool prune);

    [[nodiscard]] toolkit::result<> Update(
        Config &config,
        http::HttpClient &client);

    [[nodiscard]] toolkit::result<> Update(
        Config &config,
        http::HttpClient &client,
        std::string_view name);

    [[nodiscard]] toolkit::result<> Remove(
        Config &config,
        http::HttpClient &client,
        std::string_view version);

    [[nodiscard]] toolkit::result<> Use(
        Config &config,
        http::HttpClient &client,
        std::string_view version,
        bool local);

    [[nodiscard]] toolkit::result<> List(
        const Config &config,
        http::HttpClient &client,
        bool available,
        bool flat,
        bool details);

    [[nodiscard]] toolkit::result<> ListTracks(
        const Config &config,
        http::HttpClient &client,
        bool flat);

    [[nodiscard]] toolkit::result<> Complete(
        const Config &config,
        http::HttpClient &client,
        const toolkit::arg_context &args);

    [[nodiscard]] toolkit::result<> Execute(
        Config &config,
        http::HttpClient &client,
        std::string_view version,
        bool yes,
        const toolkit::arg_context &context);
}
