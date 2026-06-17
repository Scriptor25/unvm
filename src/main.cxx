#include <unvm/config.hxx>
#include <unvm/semver.hxx>
#include <unvm/unvm.hxx>
#include <unvm/util.hxx>
#include <unvm/http/http.hxx>

#include <toolkit/args.hxx>

#include <filesystem>
#include <iostream>

#if defined(SYSTEM_LINUX) || defined(SYSTEM_ANDROID) || defined(SYSTEM_DARWIN)
#include <unistd.h>
#endif

#if defined(SYSTEM_WINDOWS)
#include <process.h>
#include <windows.h>
#endif

enum class Operation
{
    Install,
    Remove,
    Use,
    List,
    Complete,
};

static const std::map<std::string_view, Operation> operation_map
{
    { "install", Operation::Install },
    { "i", Operation::Install },
    { "remove", Operation::Remove },
    { "r", Operation::Remove },
    { "use", Operation::Use },
    { "u", Operation::Use },
    { "list", Operation::List },
    { "l", Operation::List },
    { "complete", Operation::Complete },
    { "c", Operation::Complete },
};

static const toolkit::arg_manifest manifest
{
    {
        {
            .id = "help",
            .kind = toolkit::arg_kind::flag,
            .patterns = { "?", "-?", "-h", "--help" },
        },
        {
            .id = "local",
            .kind = toolkit::arg_kind::flag,
            .patterns = { "-l", "--local" },
        },
        {
            .id = "available",
            .kind = toolkit::arg_kind::flag,
            .patterns = { "-a", "--available" },
        },
        {
            .id = "flat",
            .kind = toolkit::arg_kind::flag,
            .patterns = { "-f", "--flat" },
        },
    },
};

static toolkit::result<> execute(
    unvm::Config &config,
    unvm::http::HttpClient &client,
    const int argc,
    const char *const*argv)
{
    toolkit::arg_context args;
    if (auto res = toolkit::arg_parse(manifest, argc, argv) >> args; !res)
        return res;

    if (args.empty() || args.is("help"))
    {
        unvm::PrintManual();
        return {};
    }

    const auto it = operation_map.find(args[0]);
    if (it == operation_map.end())
    {
        return toolkit::make_error("undefined operation '{}'.", args[0]);
    }

    switch (it->second)
    {
    case Operation::Install:
        if (args.size() != 2)
        {
            return toolkit::make_error("invalid argument count.");
        }

        return Install(config, client, args[1]);

    case Operation::Remove:
        if (args.size() != 2)
        {
            return toolkit::make_error("invalid argument count.");
        }

        return Remove(config, client, args[1]);

    case Operation::Use:
        if (args.size() != 2)
        {
            return toolkit::make_error("invalid argument count.");
        }

        return Use(config, client, args[1], args.is("local"));

    case Operation::List:
    {
        if (args.size() != 1)
        {
            return toolkit::make_error("invalid argument count.");
        }

        const auto available = args.is("available");
        const auto flat = args.is("flat");

        return List(config, client, available, flat);
    }

    case Operation::Complete:
    {
        std::vector<const char *> line(args.size());
        line[0] = args.file.data();
        for (size_t i = 1; i < args.size(); ++i)
            line[i] = args[i].data();

        toolkit::arg_context context;
        if (auto res = toolkit::arg_parse(manifest, static_cast<int>(line.size()), line.data()) >> context; !res)
            return res;

        return unvm::Complete(config, client, context);
    }

    default:
        return toolkit::make_error("operation '{}' not implemented.", args[0]);
    }
}

static void print_file_tree(const std::filesystem::path &path, const unsigned depth = {})
{
    std::cerr << std::string(depth * 2, ' ') << "+- " << path.filename().string() << std::endl;

    if (is_directory(path))
        for (auto &entry : std::filesystem::directory_iterator(path))
            print_file_tree(entry.path(), depth + 1);
}

#if defined(SYSTEM_WINDOWS)

static int execvp(const char *file, char *const argv[])
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

static int shim(const std::string &version, const std::filesystem::path &exec, const int argc, char **argv)
{
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
    args.reserve(argc + 3);

    const auto stem = exec.stem().string();

    if (stem == "node")
    {
        args.push_back(const_cast<char *>(node_path_str.c_str()));

        for (size_t i = 1; i < argc; ++i)
        {
            args.push_back(argv[i]);
        }
    }
    else if (stem == "npm")
    {
        args.push_back(const_cast<char *>(node_path_str.c_str()));
        args.push_back(const_cast<char *>(npm_cli_path_str.c_str()));

        for (size_t i = 1; i < argc; ++i)
        {
            args.push_back(argv[i]);
        }
    }
    else if (stem == "npx")
    {
        args.push_back(const_cast<char *>(node_path_str.c_str()));
        args.push_back(const_cast<char *>(npx_cli_path_str.c_str()));

        for (size_t i = 1; i < argc; ++i)
        {
            args.push_back(argv[i]);
        }
    }
    else
    {
        std::cerr << "unsupported shim target '" << stem << "'." << std::endl;
        return 1;
    }

    args.push_back(nullptr);

    const auto error = execvp(node_path_str.c_str(), args.data());

    std::cerr << "failed to execute '" << node_path_str << "': " << error << "." << std::endl;
    return 1;
}

int main(const int argc, char **argv)
{
    const auto exec = std::filesystem::path(argv[0]);

    unvm::Config config;
    unvm::http::HttpClient client;

    if (auto res = unvm::ReadConfigFile(config); !res)
    {
        std::cerr << res.error() << std::endl;
        return 1;
    }

    std::optional<std::string> maybe_active;
    unvm::VersionType type{};
    if (auto res = unvm::FindActiveVersion(maybe_active, &type); !res)
    {
        std::cerr << res.error() << std::endl;
        return 1;
    }

    if (type == unvm::VersionType::Default && config.Default)
    {
        maybe_active = *config.Default;
    }

    if (maybe_active)
    {
        unvm::semver::RangeSet set;
        if (auto res = unvm::semver::ParseRangeSet(*maybe_active) >> set; !res)
        {
            std::cerr << res.error() << std::endl;
            return 1;
        }

        unvm::VersionTable table;
        if (auto res = unvm::LoadVersionTable(client, table, false); !res)
        {
            std::cerr << res.error() << std::endl;
            return 1;
        }

        for (auto &entry : table)
        {
            if (!config.Installed.contains(entry.Version))
            {
                continue;
            }

            if (auto res = unvm::semver::IsInRange(set, entry.Version); res && !*res)
            {
                continue;
            }
            else if (!res)
            {
                std::cerr << res.error() << std::endl;
                return 1;
            }

            config.Active = entry.Version;
            break;
        }
    }

    if (exec.stem() == "unvm")
    {
        if (auto res = execute(config, client, argc, argv); !res)
        {
            std::cerr << res.error() << std::endl;
            return 1;
        }

        if (auto res = unvm::WriteConfigFile(config); !res)
        {
            std::cerr << res.error() << std::endl;
            return 1;
        }

        return 0;
    }

    if (!maybe_active)
    {
        std::cerr << "node is not active in the current context." << std::endl;
        return 1;
    }

    if (!config.Active)
    {
        unvm::semver::RangeSet set;
        if (auto res = unvm::semver::ParseRangeSet(*maybe_active) >> set; !res)
        {
            std::cerr << res.error() << std::endl;
            return 1;
        }

        unvm::VersionTable table;
        if (auto res = unvm::LoadVersionTable(client, table, true); !res)
        {
            std::cerr << res.error() << std::endl;
            return 1;
        }

        for (auto &entry : table)
        {
            if (auto res = unvm::semver::IsInRange(set, entry.Version); res && !*res)
            {
                continue;
            }
            else if (!res)
            {
                std::cerr << res.error() << std::endl;
                return 1;
            }

            if (auto res = unvm::Install(config, client, *maybe_active, entry); !res)
            {
                std::cerr << res.error() << std::endl;
                return 1;
            }

            config.Active = entry.Version;
            break;
        }

        if (!config.Active)
        {
            std::cerr << "no version matching '" << *config.Active << "'." << std::endl;
            return 1;
        }
    }

    if (auto res = unvm::WriteConfigFile(config); !res)
    {
        std::cerr << res.error() << std::endl;
        return 1;
    }

    return shim(*config.Active, exec, argc, argv);
}
