#include <unvm/semver.hxx>
#include <unvm/unvm.hxx>
#include <unvm/util.hxx>

#if defined(SYSTEM_LINUX) || defined(SYSTEM_ANDROID) || defined(SYSTEM_DARWIN)
#include <unistd.h>
#endif

#if defined(SYSTEM_WINDOWS)
#include <process.h>
#include <windows.h>
#endif

static void print_file_tree(const std::filesystem::path &path, const unsigned depth = {})
{
    std::cerr << std::string(depth * 2, ' ') << "+- " << path.filename().string() << std::endl;

    if (is_directory(path))
        for (auto &entry : std::filesystem::directory_iterator(path))
            print_file_tree(entry.path(), depth + 1);
}

#if defined(SYSTEM_WINDOWS)

static int execvp(const char *file, char **argv)
{
    std::string line;
    for (size_t i = 0; argv[i]; ++i)
    {
        std::string_view arg = argv[i];

        if (arg.find(' ') != std::string_view::npos)
        {
            line += '"';
            line += arg;
            line += '"';
        }
        else
        {
            line += arg;
        }

        if (argv[i + 1])
        {
            line += ' ';
        }
    }

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    std::vector<char> buf(line.begin(), line.end());
    buf.push_back(0);

    auto ok = CreateProcessA(nullptr, buf.data(), nullptr, nullptr, false, 0, nullptr, nullptr, &si, &pi);

    if (!ok)
    {
        std::cerr << "CreateProcess failed: " << GetLastError() << std::endl;
        return 1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD code = 1;
    GetExitCodeProcess(pi.hProcess, &code);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    ExitProcess(code);
}

#endif

static toolkit::result<> shim(const std::string &version, const toolkit::arg_context &context)
{
    std::filesystem::path exec(context.file);

    const auto data_directory = unvm::GetDataDirectory();

#if defined(SYSTEM_LINUX) || defined(SYSTEM_ANDROID) || defined(SYSTEM_DARWIN)

    const auto node_path = data_directory / version / "bin" / "node";
    const auto npm_cli_path = data_directory / version / "lib" / "node_modules" / "npm" / "bin" / "npm-cli.js";
    const auto npx_cli_path = data_directory / version / "lib" / "node_modules" / "npm" / "bin" / "npx-cli.js";

#elif defined(SYSTEM_WINDOWS)

    const auto node_path = data_directory / version / "node.exe";
    const auto npm_cli_path = data_directory / version / "node_modules" / "npm" / "bin" / "npm-cli.js";
    const auto npx_cli_path = data_directory / version / "node_modules" / "npm" / "bin" / "npx-cli.js";

#endif

    const auto node_path_str = node_path.string();
    const auto npm_cli_path_str = npm_cli_path.string();
    const auto npx_cli_path_str = npx_cli_path.string();

    std::vector<char *> args;
    args.push_back(const_cast<char *>(node_path_str.c_str()));

    const auto stem = exec.stem().string();

    if (stem == "node")
    {
    }
    else if (stem == "npm")
    {
        args.push_back(const_cast<char *>(npm_cli_path_str.c_str()));
    }
    else if (stem == "npx")
    {
        args.push_back(const_cast<char *>(npx_cli_path_str.c_str()));
    }
    else
    {
        return toolkit::make_error("unsupported shim target '{}'.", stem);
    }

    std::vector<std::string> data(context.size());
    for (size_t i = 0; i < context.size(); ++i)
    {
        data[i] = context[i];
    }

    for (auto &arg : data)
    {
        args.push_back(const_cast<char *>(arg.c_str()));
    }

    args.push_back(nullptr);

    const auto error = execvp(node_path_str.c_str(), args.data());

    return toolkit::make_error("failed to execute '{}': {}", node_path_str, error);
}

static toolkit::result<const unvm::VersionEntry *> find_version_entry(
    const unvm::Config &config,
    unvm::http::HttpClient &client,
    const std::string_view version,
    const bool online)
{
    unvm::VersionTable table;
    if (auto res = LoadVersionTable(client, table, online); !res)
    {
        return res;
    }

    FilterVersionTable(config, table, true, false);

    if (auto *effective = FindEffectiveVersion(table, version))
    {
        return effective;
    }

    unvm::semver::RangeSet set;
    if (auto res = unvm::semver::ParseRangeSet(version) >> set; !res)
    {
        return res;
    }

    for (auto &entry : table)
    {
        bool in_range;
        if (auto res = IsInRange(set, entry.Version) >> in_range; !res)
        {
            return res;
        }

        if (!in_range)
        {
            continue;
        }

        return &entry;
    }

    return nullptr;
}

toolkit::result<> unvm::Execute(
    Config &config,
    http::HttpClient &client,
    std::string_view version,
    const bool yes,
    const toolkit::arg_context &context)
{
    std::cerr << "B" << std::endl;

    const VersionEntry *version_entry{};
    if (auto res = find_version_entry(config, client, version, false) >> version_entry; !res)
    {
        return res;
    }

    std::cerr << "C" << std::endl;

    if (!version_entry)
    {
        if (auto res = find_version_entry(config, client, version, true) >> version_entry; !res)
        {
            return res;
        }
    }

    std::cerr << "D" << std::endl;

    if (!version_entry)
    {
        return toolkit::make_error("no version matching '{}'.", version);
    }

    std::cerr << "E" << std::endl;

    config.Active = version_entry->Version;

    if (!config.Installed.contains(version_entry->Version))
    {
        if (!yes)
        {
            std::cout << "version '" << version << "' is not installed." << std::endl;

            if (!Confirm("install this version?"))
            {
                return toolkit::make_error("version '{}' is not installed.", version);
            }
        }

        if (auto res = Install(config, client, version, *version_entry); !res)
        {
            return res;
        }

        if (auto res = WriteConfigFile(config); !res)
        {
            return res;
        }
    }

    std::cerr << "F" << std::endl;

    return shim(*config.Active, context);
}
