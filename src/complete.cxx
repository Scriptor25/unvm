#include <unvm/unvm.hxx>

#include <toolkit/string.hxx>

#include <iostream>

toolkit::result<> unvm::Complete(const Config &config, const http::client &client, const args::context &args)
{
    if (args.limited())
    {
        std::cout << "";
        return {};
    }

    std::cout << "-- ";

    if (!args.is("help"))
    {
        std::cout << "? -? -h --help ";
    }

    // root
    if (args.empty())
    {
        std::cout << "i install r remove u use l list c complete x e exec execute t track n untrack p update";
        return {};
    }

    // install (latest|lts|<version>)
    if (args[0] == "i" || args[0] == "install")
    {
        if (args.size() == 1)
        {
            std::cout << "latest lts ";
            if (auto res = List(config, client, true, true, false); !res)
            {
                return res;
            }
        }

        return {};
    }

    // remove (latest|lts|<version>)
    if (args[0] == "r" || args[0] == "remove")
    {
        if (args.size() == 1)
        {
            std::cout << "latest lts ";

            if (auto res = List(config, client, false, true, false); !res)
            {
                return res;
            }
        }

        return {};
    }

    // use (none|latest|lts|<version>) [(-l|--local)]
    if (args[0] == "u" || args[0] == "use")
    {
        if (!args.is("local"))
        {
            std::cout << "-l --local ";
        }

        if (args.size() == 1)
        {
            std::cout << "none latest lts ";

            if (auto res = List(config, client, false, true, false); !res)
            {
                return res;
            }
        }

        return {};
    }

    // list [(-a|--available|-f|--flat|-d|--details)]...
    if (args[0] == "l" || args[0] == "list")
    {
        if (!args.is("available"))
        {
            std::cout << "-a --available ";
        }

        if (!args.is("flat"))
        {
            std::cout << "-f --flat ";
        }

        if (!args.is("details"))
        {
            std::cout << "-d --details ";
        }

        return {};
    }

    // complete -- ...
    if (args[0] == "c" || args[0] == "complete")
    {
        return {};
    }

    // execute [<version>] [(-y|--yes)] -- ...
    if (args[0] == "x" || args[0] == "e" || args[0] == "exec" || args[0] == "execute")
    {
        if (!args.is("yes"))
        {
            std::cout << "-y --yes ";
        }

        if (args.size() == 1)
        {
            std::cout << "latest lts ";

            if (auto res = List(config, client, false, true, false); !res)
            {
                return res;
            }
        }

        return {};
    }

    if (args[0] == "t" || args[0] == "track")
    {
        if (args.size() == 1)
        {
            std::cout << "latest lts ";

            VersionTable table;
            if (auto res = LoadVersionTable(client, true) >> table; !res)
            {
                return res;
            }

            FilterVersionTable(config, table, true);

            std::unordered_set<std::string> versions;

            for (auto &entry : table)
            {
                if (entry.LTS)
                {
                    versions.insert(*entry.LTS);
                    versions.insert(toolkit::lowercase(*entry.LTS));
                }
            }

            for (auto &version : versions)
            {
                std::cout << version << ' ';
            }
        }

        return {};
    }

    if (args[0] == "n" || args[0] == "untrack")
    {
        if (args.size() == 1)
        {
            for (auto &tag : config.Tracked)
            {
                std::cout << tag << ' ';
            }
        }

        return {};
    }

    if (args[0] == "p" || args[0] == "update")
    {
        if (args.size() == 1)
        {
            for (auto &tag : config.Tracked)
            {
                std::cout << tag << ' ';
            }
        }

        return {};
    }

    std::cout << "";
    return {};
}
