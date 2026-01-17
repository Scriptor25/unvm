#include <util.hxx>

std::istream &GetLine(std::istream &stream, std::string &string, const std::string_view &delim)
{
    string.clear();

    while (stream.good() && string.find(delim) == std::string::npos)
        string += static_cast<char>(stream.get());

    if (string.find(delim) == std::string::npos)
        return stream;

    string = string.substr(0, string.size() - delim.size());
    return stream;
}

std::string Trim(std::string string)
{
    if (string.empty())
        return string;

    for (auto it = string.begin(); it != string.end(); ++it)
    {
        if (!std::isspace(*it))
            break;

        string.erase(it);
    }

    for (auto it = string.rbegin(); it != string.rend(); ++it)
    {
        if (!std::isspace(*it))
            break;

        string.erase(it.base());
    }

    return string;
}

std::string Lower(std::string string)
{
    for (auto &it : string)
        it = static_cast<char>(std::tolower(it));
    return string;
}
