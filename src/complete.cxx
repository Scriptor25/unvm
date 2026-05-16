#include <unvm/unvm.hxx>

#include <iostream>

toolkit::result<> unvm::Complete(const Config &config, http::HttpClient &client, const toolkit::arg_context &args)
{
    if (!args.is("help"))
    {
        std::cout << "? -? -h --help ";
    }

    // root
    if (args.empty())
    {
        std::cout << "i install r remove u use l list c complete";
        return {};
    }

    // install latest|lts|<version>
    if (args[0] == "i" || args[0] == "install")
    {
        if (args.size() == 1)
        {
            std::cout << "latest lts ";
            return List(config, client, true, true);
        }

        return {};
    }

    // remove latest|lts|<version>
    if (args[0] == "r" || args[0] == "remove")
    {
        if (args.size() == 1)
        {
            std::cout << "latest lts ";
            return List(config, client, false, true);
        }

        return {};
    }

    // use none|latest|lts|<version> -l|--local
    if (args[0] == "u" || args[0] == "use")
    {
        if (!args.is("local"))
        {
            std::cout << "-l --local ";
        }

        if (args.size() == 1)
        {
            std::cout << "none latest lts ";
            return List(config, client, false, true);
        }

        return {};
    }

    // list (-a|--available|-f|--flat)...
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

        return {};
    }

    // complete -- ...
    if (args[0] == "c" || args[0] == "complete")
    {
        return {};
    }

    std::cout << "";
    return {};
}
