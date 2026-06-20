#pragma once

#include <cstdint>
#include <format>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace unvm
{
    struct VersionEntry
    {
        std::string Version;
        std::string Date;
        std::set<std::string> Files;
        std::optional<std::string> NPM;
        std::string V8;
        std::optional<std::string> UV;
        std::optional<std::string> ZLib;
        std::optional<std::string> OpenSSL;
        uint16_t Modules;
        std::optional<std::string> LTS;
        bool Security{};
    };

    using VersionTable = std::vector<VersionEntry>;

    struct Platform
    {
        std::format_string<const std::string &> Format;
        std::string_view Extension;
        std::string_view Pattern;
    };

#if defined(SYSTEM_WINDOWS)

#if defined(ARCH_X86)
    constexpr Platform platform
    {
        .Format = "node-{}-win-x86",
        .Extension = "zip",
        .Pattern = "win-x86-zip",
    };
#endif

#if defined(ARCH_X86_64) || defined(ARCH_AMD64)
    constexpr Platform platform
    {
        .Format = "node-{}-win-x64",
        .Extension = "zip",
        .Pattern = "win-x64-zip",
    };
#endif

#if defined(ARCH_ARM64)
    constexpr Platform platform
    {
        .Format = "node-{}-win-arm64",
        .Extension = "zip",
        .Pattern = "win-arm64-zip",
    };
#endif

#endif

#if defined(SYSTEM_LINUX) || defined(SYSTEM_ANDROID)

#if defined(ARCH_X86)
    constexpr Platform platform
    {
        .Format = "node-{}-linux-x86",
        .Extension = "tar.gz",
        .Pattern = "linux-x86",
    };
#endif

#if defined(ARCH_X86_64) || defined(ARCH_AMD64)
    constexpr Platform platform
    {
        .Format = "node-{}-linux-x64",
        .Extension = "tar.gz",
        .Pattern = "linux-x64",
    };
#endif

#if defined(ARCH_ARM64) || defined(ARCH_AARCH64)
    constexpr Platform platform
    {
        .Format = "node-{}-linux-arm64",
        .Extension = "tar.gz",
        .Pattern = "linux-arm64",
    };
#endif

#endif

#if defined(SYSTEM_DARWIN)

#if defined(ARCH_X86_64) || defined(ARCH_AMD64)
    constexpr Platform platform
    {
        .Format = "node-{}-darwin-x64",
        .Extension = "tar.gz",
        .Pattern = "osx-x64-tar",
    };
#endif

#if defined(ARCH_ARM64)
    constexpr Platform platform
    {
        .Format = "node-{}-darwin-arm64",
        .Extension = "tar.gz",
        .Pattern = "osx-arm64-tar",
    };
#endif

#endif
}
