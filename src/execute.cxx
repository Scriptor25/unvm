#include <unvm/lock.hxx>
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

[[nodiscard]] static toolkit::result<> shim(const std::string &version, const toolkit::arg_context &context)
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

[[nodiscard]] static toolkit::result<> load_filter_table(
    const unvm::Config &config,
    unvm::http::HttpClient &client,
    unvm::VersionTable &table,
    const bool online)
{
    if (auto res = LoadVersionTable(client, table, online); !res)
    {
        return res;
    }

    FilterVersionTable(config, table, true);
    return {};
}

toolkit::result<> unvm::Execute(
    Config &config,
    http::HttpClient &client,
    std::string_view version,
    const bool yes,
    const toolkit::arg_context &context)
{
    const VersionEntry *entry{};

    VersionTable table;
    if (auto res = load_filter_table(config, client, table, false); !res)
    {
        return res;
    }

    if (auto res = FindVersionEntry(table, version) >> entry; !res)
    {
        return res;
    }

    if (!entry)
    {
        if (auto res = load_filter_table(config, client, table, true); !res)
        {
            return res;
        }

        if (auto res = FindVersionEntry(table, version) >> entry; !res)
        {
            return res;
        }
    }

    if (!entry)
    {
        return toolkit::make_error("no version matching '{}'.", version);
    }

    {
        const auto data_directory = GetDataDirectory();
        const auto lock_path = data_directory / (entry->Version + ".lock");

        TryAcquire lock(lock_path, true, "install");
        if (!lock.Primary())
        {
            if (auto res = ReloadConfigFile(config); !res)
            {
                return res;
            }
        }

        if (!config.Installed.contains(entry->Version))
        {
            if (!yes)
            {
                std::cout << "version '" << version << "' is not installed." << std::endl;

                if (!Confirm("install this version?"))
                {
                    return toolkit::make_error("version '{}' is not installed.", version);
                }
            }

            if (auto res = Install(config, client, version, *entry); !res)
            {
                return res;
            }

            if (auto res = WriteConfigFile(config); !res)
            {
                return res;
            }
        }

        config.Active = entry->Version;
    }

    return shim(entry->Version, context);
}
