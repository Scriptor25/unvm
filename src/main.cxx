#include <unvm/config.hxx>
#include <unvm/json.hxx>
#include <unvm/unvm.hxx>
#include <unvm/util.hxx>
#include <unvm/http/http.hxx>

#include <filesystem>
#include <fstream>
#include <iostream>

#if defined(SYSTEM_LINUX) || defined(SYSTEM_ANDROID) || defined(SYSTEM_DARWIN)
#include <unistd.h>
#endif

#if defined(SYSTEM_WINDOWS)
#include <windows.h>
#endif

constexpr auto MOD_PRESENT_BITS = 0b1000000000000000u;
constexpr auto MOD_VALUE_BITS = 0b0111000000000000u;
constexpr auto OPERATION_BITS = 0b0000000011111111u;

constexpr auto INSTALL_BITS = 0b00000001u;
constexpr auto REMOVE_BITS = 0b00000010u;
constexpr auto USE_BITS = 0b00000100u;
constexpr auto LIST_BITS = 0b00001000u;
constexpr auto WORKSPACE_BITS = 0b00010000u;

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
    { "workspace", WORKSPACE_BITS },
    { "w", WORKSPACE_BITS },
};

static int execute(const std::vector<std::string_view> &args)
{
    if (args.empty())
    {
        unvm::PrintManual();
        return 0;
    }

    if (!operation_map.contains(args[0]))
    {
        std::cerr << "undefined operation '" << args[0] << "'." << std::endl;
        return 1;
    }

    auto operation = operation_map.at(args[0]);

    auto data_directory = unvm::GetDataDirectory();

    unvm::Config config;
    if (std::ifstream stream(data_directory / "config.json"); stream)
    {
        json::Node json;
        stream >> json;

        if (!(json >> config))
        {
            std::cerr << "failed to parse config json." << std::endl;
            return 1;
        }
    }

    unvm::http::HttpClient client;

    int code;
    switch (operation & OPERATION_BITS)
    {
    case INSTALL_BITS:
        if (args.size() != 2)
        {
            std::cerr << "invalid argument count." << std::endl;
            return 1;
        }
        code = Install(config, client, args[1]);
        break;

    case REMOVE_BITS:
        if (args.size() != 2)
        {
            std::cerr << "invalid argument count." << std::endl;
            return 1;
        }
        code = Remove(config, client, args[1]);
        break;

    case USE_BITS:
        switch (args.size())
        {
        case 2:
            code = Use(config, client, args[1], false);
            break;
        case 3:
            if (args[2] != "local")
            {
                std::cerr << "invalid use modifier '" << args[2] << "'." << std::endl;
                return 1;
            }
            code = Use(config, client, args[1], true);
            break;
        default:
            std::cerr << "invalid argument count." << std::endl;
            return 1;
        }
        break;

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

        code = List(config, client, available);
        break;
    }

    case WORKSPACE_BITS:
        if (args.size() != 1)
        {
            std::cerr << "invalid argument count." << std::endl;
            return 1;
        }
        code = Workspace(config, client);
        break;

    default:
        code = 1;
        break;
    }

    {
        std::filesystem::create_directories(data_directory);

        std::ofstream stream(data_directory / "config.json");
        if (!stream)
        {
            std::cerr << "failed to open config." << std::endl;
            return 1;
        }

        stream << json::Node(config);
    }

    return code;
}

int main(const int argc, char **argv)
{
    auto arg0 = std::filesystem::path(argv[0]);
    if (arg0.stem() == "unvm" || arg0.stem() == "unvm.exe")
        return execute({ argv + 1, argv + argc });

    std::optional<std::string> version;
    if (auto error = unvm::LoadLocalVersion(version))
        return error;

    auto data_directory = unvm::GetDataDirectory();

    if (!version)
    {
        unvm::Config config;
        if (std::ifstream stream(data_directory / "config.json"); stream)
        {
            json::Node json;
            stream >> json;

            if (!(json >> config))
            {
                std::cerr << "failed to parse config json." << std::endl;
                return 1;
            }
        }

        if (config.Default)
            version = *config.Default;
    }

    if (!version)
    {
        std::cerr << "node is not active." << std::endl;
        return 1;
    }

    auto file_path = data_directory / *version / "bin" / arg0.filename();
    auto file_path_str = file_path.string();

    std::vector<char *> args{ argv, argv + argc };
    args[0] = file_path_str.data();
    args.push_back(nullptr);

    std::cerr << "node version " << *version << ", path '" << file_path_str << "'." << std::endl;

#if defined(SYSTEM_LINUX) || defined(SYSTEM_ANDROID) || defined(SYSTEM_DARWIN)
    execvp(file_path.c_str(), args.data());
#endif

#if defined(SYSTEM_WINDOWS)
    #error not supported yet
#endif

    return 1;
}
