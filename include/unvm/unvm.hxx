#pragma once

#include <unvm/config.hxx>
#include <unvm/http.hxx>
#include <unvm/version.hxx>

#include <args/args.hxx>
#include <toolkit/result.hxx>

#include <filesystem>
#include <string_view>

namespace unvm
{
    void PrintManual();

    [[nodiscard]] toolkit::result<> LoadVersionTable(
        const http::client &client,
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
        const http::client &client,
        std::string_view version,
        const VersionEntry &entry);
    [[nodiscard]] toolkit::result<> Install(
        Config &config,
        const http::client &client,
        std::string_view version);

    [[nodiscard]] toolkit::result<> Remove(
        Config &config,
        const http::client &client,
        std::string_view version);

    [[nodiscard]] toolkit::result<> Use(
        Config &config,
        const http::client &client,
        std::string_view version,
        bool local);

    [[nodiscard]] toolkit::result<> List(
        const Config &config,
        const http::client &client,
        bool available,
        bool flat,
        bool details);

    [[nodiscard]] toolkit::result<> Complete(
        const Config &config,
        const http::client &client,
        const args::context &args);

    [[nodiscard]] toolkit::result<> Execute(
        Config &config,
        const http::client &client,
        std::string_view version,
        bool yes,
        const args::context &args);
}
