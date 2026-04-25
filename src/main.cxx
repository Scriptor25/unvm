#include <unvm/config.hxx>
#include <unvm/semver.hxx>
#include <unvm/unvm.hxx>
#include <unvm/util.hxx>
#include <unvm/http/http.hxx>

#include <cstring>
#include <filesystem>
#include <iostream>

#if defined(SYSTEM_LINUX) || defined(SYSTEM_ANDROID) || defined(SYSTEM_DARWIN)
#include <unistd.h>
#endif

#if defined(SYSTEM_WINDOWS)
#include <process.h>
#include <windows.h>
#endif

constexpr auto MOD_PRESENT_BITS = 0b1000000000000000u;
constexpr auto MOD_VALUE_BITS = 0b0111000000000000u;
constexpr auto OPERATION_BITS = 0b0000000011111111u;

constexpr auto INSTALL_BITS = 0b00000001u;
constexpr auto REMOVE_BITS = 0b00000010u;
constexpr auto USE_BITS = 0b00000100u;
constexpr auto LIST_BITS = 0b00001000u;

constexpr auto LIST_MOD_INSTALLED_BITS = 0b0000000000000000u;
constexpr auto LIST_MOD_AVAILABLE_BITS = 0b0001000000000000u;

/**
 * bits [3:0] -> operation
 * bits [7:4] -> modifier
 *  - bit 7 -> modifier present bit
 *  - bits [6:4] -> modifier value
 */
static const std::map<std::string_view, unsigned> operation_map
{
    { "install", INSTALL_BITS },
    { "i", INSTALL_BITS },
    { "remove", REMOVE_BITS },
    { "r", REMOVE_BITS },
    { "use", USE_BITS },
    { "u", USE_BITS },
    { "list", LIST_BITS },
    { "l", LIST_BITS },
    { "ls", MOD_PRESENT_BITS | LIST_MOD_INSTALLED_BITS | LIST_BITS },
    { "la", MOD_PRESENT_BITS | LIST_MOD_AVAILABLE_BITS | LIST_BITS },
};

static int execute(unvm::Config &config, unvm::http::HttpClient &client, const std::vector<std::string_view> &args)
{
    if (args.empty())
    {
        unvm::PrintManual();
        return 0;
    }

    const auto it = operation_map.find(args[0]);
    if (it == operation_map.end())
    {
        std::cerr << "undefined operation '" << args[0] << "'." << std::endl;
        return 1;
    }

    switch (const auto operation = it->second; operation & OPERATION_BITS)
    {
    case INSTALL_BITS:
        if (args.size() != 2)
        {
            std::cerr << "invalid argument count." << std::endl;
            return 1;
        }
        return Install(config, client, args[1]);

    case REMOVE_BITS:
        if (args.size() != 2)
        {
            std::cerr << "invalid argument count." << std::endl;
            return 1;
        }
        return Remove(config, client, args[1]);

    case USE_BITS:
        switch (args.size())
        {
        case 2:
            return Use(config, client, args[1], false);
        case 3:
            if (args[2] != "local")
            {
                std::cerr << "invalid use modifier '" << args[2] << "'." << std::endl;
                return 1;
            }
            return Use(config, client, args[1], true);
        default:
            std::cerr << "invalid argument count." << std::endl;
            return 1;
        }

    case LIST_BITS:
    {
        bool available;
        if (operation & MOD_PRESENT_BITS)
        {
            if (args.size() != 1)
            {
                std::cerr << "invalid argument count." << std::endl;
                return 1;
            }
            available = (operation & MOD_VALUE_BITS) == LIST_MOD_AVAILABLE_BITS;
        }
        else
        {
            switch (args.size())
            {
            case 1:
                available = false;
                break;

            case 2:
                if (args[1] != "available")
                {
                    std::cerr << "invalid list modifier '" << args[1] << "'." << std::endl;
                    return 1;
                }
                available = true;
                break;

            default:
                std::cerr << "invalid argument count." << std::endl;
                return 1;
            }
        }

        return List(config, client, available);
    }

    default:
        std::cerr << "operation not implemented." << std::endl;
        return 1;
    }
}

static int execute(const std::string &version, const std::filesystem::path &exec, const int argc, char **argv)
{
    const auto data_directory = unvm::GetDataDirectory();

    const auto exec_path = data_directory / version / "bin" / exec.filename();
    const auto exec_path_str = exec_path.string();

#if defined(SYSTEM_LINUX) || defined(SYSTEM_ANDROID) || defined(SYSTEM_DARWIN)

    std::vector<char *> args{ argv, argv + argc };
    args[0] = const_cast<char *>(exec_path_str.c_str());
    args.push_back(nullptr);

    execvp(exec_path_str.c_str(), args.data());

    std::cerr << "failed to execute '" << exec_path_str << "': " << std::strerror(errno) << "." << std::endl;
    return 1;

#endif

#if defined(SYSTEM_WINDOWS)

    auto cmdline = '"' + exec_path_str + '"';
    for (auto i = 1; i < argc; ++i)
    {
        cmdline += " \"";
        cmdline += argv[i];
        cmdline += "\"";
    }

    STARTUPINFOA si{};
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi{};

    auto pwd = exec_path.parent_path().string();

    auto success = CreateProcessA(
        exec_path_str.c_str(),
        cmdline.c_str(),
        nullptr,
        nullptr,
        false,
        0,
        nullptr,
        nullptr,
        &si,
        &pi);

    if (!success)
    {
        auto err = GetLastError();
        std::cerr << "failed to execute '" << exec_path_str << "': " << err << std::endl;
        return 1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD code{};
    GetExitCodeProcess(pi.hProcess, &code);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return static_cast<int>(code);

#endif
}

int main(const int argc, char **argv)
{
    const auto exec = std::filesystem::path(argv[0]);

    unvm::Config config;
    unvm::http::HttpClient client;

    if (const auto error = unvm::ReadConfigFile(config))
    {
        return error;
    }

    std::optional<std::string> maybe_active;
    if (const auto error = unvm::FindActiveVersion(maybe_active))
    {
        return error;
    }

    if (!maybe_active && config.Default)
    {
        maybe_active = *config.Default;
    }

    if (maybe_active)
    {
        const auto set = unvm::semver::ParseRangeSet(*maybe_active);

        unvm::VersionTable table;
        if (const auto error = unvm::LoadVersionTable(client, table, false))
        {
            return error;
        }

        for (auto &entry : table)
        {
            if (!config.Installed.contains(entry.Version))
            {
                continue;
            }

            if (!unvm::semver::IsInRange(set, entry.Version))
            {
                continue;
            }

            config.Active = entry.Version;
            break;
        }
    }

    if (exec.stem() == "unvm" || exec.stem() == "unvm.exe")
    {
        if (const auto error = execute(config, client, { argv + 1, argv + argc }))
        {
            return error;
        }

        return unvm::WriteConfigFile(config);
    }

    if (!maybe_active)
    {
        std::cerr << "node is not active in the current context." << std::endl;
        return 1;
    }

    if (!config.Active)
    {
        const auto set = unvm::semver::ParseRangeSet(*maybe_active);

        unvm::VersionTable table;
        if (const auto error = unvm::LoadVersionTable(client, table, true))
        {
            return error;
        }

        for (auto &entry : table)
        {
            if (!unvm::semver::IsInRange(set, entry.Version))
            {
                continue;
            }

            if (const auto error = unvm::Install(config, client, *maybe_active, entry))
            {
                return error;
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

    if (const auto error = unvm::WriteConfigFile(config))
    {
        return error;
    }

    return execute(*config.Active, exec, argc, argv);
}
