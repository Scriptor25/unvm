#include <unvm/unvm.hxx>

#include <iostream>

toolkit::result<> unvm::Complete(const Config &config, http::Client &client, const toolkit::arg_context &args)
{
    if (args.limit != ~size_t())
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
        std::cout << "i install r remove u use l list c complete x e exec execute";
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

    std::cout << "";
    return {};
}
