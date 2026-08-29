#include <unvm/config.hxx>
#include <unvm/http.hxx>
#include <unvm/semver.hxx>
#include <unvm/unvm.hxx>
#include <unvm/util.hxx>

#include <args/args.hxx>

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
    Track,
    Untrack,
    Tags,
    Update,
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
    { "track", Operation::Track },
    { "untrack", Operation::Untrack },
    { "tags", Operation::Tags },
    { "update", Operation::Update },
};

static const args::manifest manifest
{
    {
        {
            .id = "help",
            .kind = args::entry_kind::flag,
            .patterns = { "?", "-?", "-h", "--help" },
        },
        {
            .id = "local",
            .kind = args::entry_kind::flag,
            .patterns = { "-l", "--local" },
        },
        {
            .id = "available",
            .kind = args::entry_kind::flag,
            .patterns = { "-a", "--available" },
        },
        {
            .id = "flat",
            .kind = args::entry_kind::flag,
            .patterns = { "-f", "--flat" },
        },
        {
            .id = "details",
            .kind = args::entry_kind::flag,
            .patterns = { "-d", "--details" },
        },
        {
            .id = "yes",
            .kind = args::entry_kind::flag,
            .patterns = { "-y", "--yes" },
        },
    },
};

[[nodiscard]] static toolkit::result<> execute(
    unvm::Config &config,
    http::client &client,
    const int argc,
    char **argv)
{
    args::context args;
    if (auto res = args::context::parse(manifest, argc, argv) >> args; !res)
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
    {
        if (args.size() != 2)
        {
            return toolkit::make_error("invalid argument count.");
        }

        const auto local = args.is("local");

        return Use(config, client, args[1], local);
    }

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
        line[0] = args.file().data();
        for (size_t i = 1; i < args.size(); ++i)
            line[i] = args[i].data();

        args::context context;
        if (auto res = args::context::parse(manifest, static_cast<int>(line.size()), line.data()) >> context; !res)
            return res;

        return unvm::Complete(config, client, context);
    }

    case Operation::Execute:
    {
        auto count = args.limited() ? args.limit() : args.size();

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

        args::context context;
        if (auto res = args::context::parse(manifest, static_cast<int>(line.size()), line.data()) >> context; !res)
            return res;

        return unvm::Execute(config, client, version, yes, context);
    }

    case Operation::Track:
    {
        if (args.size() != 2)
        {
            return toolkit::make_error("invalid argument count.");
        }

        return unvm::Track(config, client, args[1]);
    }

    case Operation::Untrack:
    {
        if (args.size() != 2)
        {
            return toolkit::make_error("invalid argument count.");
        }

        return unvm::Untrack(config, client, args[1]);
    }

    case Operation::Tags:
    {
        if (args.size() != 1)
        {
            return toolkit::make_error("invalid argument count.");
        }

        const auto available = args.is("available");
        const auto flat = args.is("flat");

        return unvm::Tags(config, client, available, flat);
    }

    case Operation::Update:
    {
        switch (args.size())
        {
        case 1:
            return unvm::Update(config, client);

        case 2:
            return unvm::Update(config, client, args[1]);

        default:
            return toolkit::make_error("invalid argument count.");
        }
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

    std::unique_ptr<http::transport> transport;
    if (auto res = unvm::CreateTransport() >> transport; !res)
    {
        std::cerr << res.error() << std::endl;
        return 1;
    }

    http::client client(*transport);

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
        if (auto res = unvm::LoadVersionTable(client, false) >> table; !res)
        {
            std::cerr << res.error() << std::endl;
            return 1;
        }

        unvm::FilterVersionTable(config, table, true, true);

        const unvm::VersionEntry *entry{};
        if (auto res = unvm::FindVersionEntry(table, *config.Detected) >> entry; !res)
        {
            std::cerr << res.error() << std::endl;
            return 1;
        }

        if (entry)
        {
            config.Active = entry->Version;
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

    args::context context;
    if (auto res = args::context::parse(manifest, argc, argv) >> context; !res)
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
