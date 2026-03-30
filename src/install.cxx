#include <unvm/unvm.hxx>

#include <iostream>

int unvm::Install(unvm::Config &config, http::Client &client, std::string_view version, const unvm::VersionEntry &entry)
{
    if (config.Installed.contains(entry.Version))
    {
        std::cerr << "version '" << version << "' is already installed." << std::endl;
        return 0;
    }

#if defined(SYSTEM_WINDOWS)

#if defined(ARCH_X86_64) || defined(ARCH_AMD64)
    constexpr auto format = "node-{}-win-x64";
#endif

#if defined(ARCH_ARM64)
    constexpr auto format = "node-{}-win-arm64";
#endif

    constexpr auto ending = "zip";

#endif

#if defined(SYSTEM_LINUX) || defined(SYSTEM_ANDROID)

#if defined(ARCH_X86_64) || defined(ARCH_AMD64)
    constexpr auto format = "node-{}-linux-x64";
#endif

#if defined(ARCH_ARM64) || defined(ARCH_AARCH64)
    constexpr auto format = "node-{}-linux-arm64";
#endif

    constexpr auto ending = "tar.xz";

#endif

#if defined(SYSTEM_DARWIN)

#if defined(ARCH_X86_64) || defined(ARCH_AMD64)
    constexpr auto format = "node-{}-darwin-x64";
#endif

#if defined(ARCH_ARM64)
    constexpr auto format = "node-{}-darwin-arm64";
#endif

    constexpr auto ending = "tar.xz";

#endif

    auto filename = std::format(format, entry.Version);
    auto pathname = std::format("/dist/{}/{}.{}", entry.Version, filename, ending);

    std::stringstream stream(std::stringstream::binary | std::stringstream::in | std::stringstream::out);

    http::Request request
    {
        .Method = http::Method::Get,
        .Location = {
            .UseTLS = true,
            .Host = "nodejs.org",
            .Port = 443,
            .Pathname = pathname,
        },
    };

    http::Response response
    {
        .Body = &stream,
    };

    if (auto error = client.Fetch(std::move(request), response))
    {
        std::cerr << "failed to get file." << std::endl;
        return error;
    }

    if (!http::IsSuccess(response.StatusCode))
    {
        std::cerr
                << "failed to get file: "
                << response.StatusCode
                << ", "
                << response.StatusMessage
                << std::endl
                << stream.str()
                << std::endl;
        return 1;
    }

    if (std::error_code ec; std::filesystem::create_directories(config.InstallDirectory, ec), ec)
    {
        std::cerr
                << "failed to create install directory '"
                << config.InstallDirectory.string()
                << "': "
                << ec.message()
                << " ("
                << ec.value()
                << ")"
                << std::endl;
        return ec.value();
    }

    if (const auto error = unvm::UnpackArchive(stream, config.InstallDirectory))
    {
        std::cerr << "failed to unpack archive." << std::endl;
        return error;
    }

    if (std::error_code ec; std::filesystem::rename(config.InstallDirectory / filename, config.InstallDirectory / entry.Version, ec), ec)
    {
        std::cerr
                << "failed to rename '"
                << (config.InstallDirectory / filename).string()
                << "' to '"
                << (config.InstallDirectory / entry.Version).string()
                << "': "
                << ec.message()
                << " ("
                << ec.value()
                << ")"
                << std::endl;
        return ec.value();
    }

    config.Installed.emplace(entry.Version);
    return 0;
}

int unvm::Install(unvm::Config &config, http::Client &client, std::string_view version)
{
    unvm::VersionTable table;
    if (auto error = unvm::LoadVersionTable(client, table, true))
    {
        std::cerr << "failed to load version table." << std::endl;
        return error;
    }

    auto entry_ptr = unvm::FindEffectiveVersion(table, version);
    if (!entry_ptr)
    {
        std::cerr << "no effective version for '" << version << "'." << std::endl;
        return 1;
    }

    return Install(config, client, version, *entry_ptr);
}
