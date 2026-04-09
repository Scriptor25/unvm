#include <unvm/util.hxx>

#include <iterator>

std::istream &unvm::GetLine(std::istream &stream, std::string &string, std::string_view delim)
{
    string.clear();

    while (stream.good() && !stream.eof() && string.find(delim) == std::string::npos)
    {
        auto c = stream.get();
        if (c < 0)
        {
            break;
        }

        string += static_cast<char>(c);
    }

    if (string.find(delim) == std::string::npos)
    {
        return stream;
    }

    string = string.substr(0, string.size() - delim.size());
    return stream;
}

std::string unvm::Trim(std::string string)
{
    if (string.empty())
    {
        return string;
    }

    std::string::iterator begin, end;

    for (auto it = string.begin(); it != string.end(); ++it)
    {
        if (*it > 0x20)
        {
            begin = it;
            break;
        }
    }

    for (auto it = string.rbegin(); it != string.rend(); ++it)
    {
        if (*it > 0x20)
        {
            end = it.base();
            break;
        }
    }

    return { std::make_move_iterator(begin), std::make_move_iterator(end) };
}

std::string unvm::Lower(std::string string)
{
    for (auto &it : string)
    {
        it = static_cast<char>(std::tolower(it));
    }
    return string;
}
