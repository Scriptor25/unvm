#include <unvm/config.hxx>
#include <unvm/semver.hxx>
#include <unvm/unvm.hxx>
#include <unvm/util.hxx>
#include <unvm/http/http.hxx>

#include <toolkit/args.hxx>

#include <filesystem>
#include <iostream>

enum class Operation
{
    Install,
    Remove,
    Use,
    List,
    Complete,
    Execute,
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
    { "execute", Operation::Execute },
    { "exec", Operation::Execute },
    { "e", Operation::Execute },
    { "x", Operation::Execute },
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
        {
            .id = "details",
            .kind = toolkit::arg_kind::flag,
            .patterns = { "-d", "--details" },
        },
        {
            .id = "yes",
            .kind = toolkit::arg_kind::flag,
            .patterns = { "-y", "--yes" },
        },
    },
};

static toolkit::result<> execute(
    unvm::Config &config,
    unvm::http::HttpClient &client,
    const int argc,
    char **argv)
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
        const auto details = args.is("details");

        return List(config, client, available, flat, details);
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

    case Operation::Execute:
    {
        auto count = args.limit != ~size_t() ? args.limit : args.size();

        if (count != 1 && count != 2)
        {
            return toolkit::make_error("invalid argument count.");
        }

        std::string_view version;
        if (count == 1)
        {
            if (!config.Detected)
            {
                return toolkit::make_error("node is not active in the current context.");
            }

            version = *config.Detected;
        }
        else
        {
            version = args[1];
        }

        auto yes = args.is("yes");

        std::vector<const char *> line(args.size() - count);
        for (auto i = count; i < args.size(); ++i)
            line[i - count] = args[i].data();

        toolkit::arg_context context;
        if (auto res = toolkit::arg_parse(manifest, static_cast<int>(line.size()), line.data()) >> context; !res)
            return res;

        return unvm::Execute(config, client, version, yes, context);
    }

    default:
        return toolkit::make_error("operation '{}' not implemented.", args[0]);
    }
}

int main(const int argc, char **argv)
{
    const auto exec = std::filesystem::path(argv[0]);
    const auto stem = exec.stem().string();

    unvm::Config config;
    unvm::http::HttpClient client;

    if (auto res = unvm::ReadConfigFile(config); !res)
    {
        std::cerr << res.error() << std::endl;
        return 1;
    }

    unvm::VersionType type{};

    if (auto res = unvm::FindActiveVersion(config.Default, &type) >> config.Detected; !res)
    {
        std::cerr << res.error() << std::endl;
        return 1;
    }

    if (config.Detected)
    {
        unvm::VersionTable table;
        if (auto res = unvm::LoadVersionTable(client, table, false); !res)
        {
            std::cerr << res.error() << std::endl;
            return 1;
        }

        unvm::FilterVersionTable(config, table, true, true);

        if (auto *effective = unvm::FindEffectiveVersion(table, *config.Detected))
        {
            config.Active = effective->Version;
        }
        else
        {
            unvm::semver::RangeSet set;
            if (auto res = unvm::semver::ParseRangeSet(*config.Detected) >> set; !res)
            {
                std::cerr << res.error() << std::endl;
                return 1;
            }

            for (auto &entry : table)
            {
                bool in_range;
                if (auto res = unvm::semver::IsInRange(set, entry.Version) >> in_range; !res)
                {
                    std::cerr << res.error() << std::endl;
                    return 1;
                }

                if (!in_range)
                {
                    continue;
                }

                config.Active = entry.Version;
                break;
            }
        }
    }

    if (stem == "unvm")
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

    if (!config.Detected)
    {
        std::cerr << "node is not active in the current context." << std::endl;
        return 1;
    }

    toolkit::arg_context context;
    if (auto res = toolkit::arg_parse(manifest, argc, argv) >> context; !res)
    {
        std::cerr << res.error() << std::endl;
        return 1;
    }

    if (auto res = unvm::Execute(config, client, *config.Detected, false, context); !res)
    {
        std::cerr << res.error() << std::endl;
        return 1;
    }

    std::cerr << "unhandled state reached." << std::endl;
    return 1;
}
