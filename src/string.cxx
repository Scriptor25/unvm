#include <unvm/util.hxx>

#include <iterator>

std::istream &unvm::GetLine(std::istream &stream, std::string &string, std::string_view delim)
{
    string.clear();

    while (stream.good() && !stream.eof() && string.find(delim) == std::string::npos)
    {
        auto c = stream.get();
        if (c < 0)
            break;

        string += static_cast<char>(c);
    }

    if (string.find(delim) == std::string::npos)
        return stream;

    string = string.substr(0, string.size() - delim.size());
    return stream;
}

std::string unvm::Trim(std::string string)
{
    if (string.empty())
        return string;

    std::string::iterator begin, end;

    for (auto it = string.begin(); it != string.end(); ++it)
        if (*it > 0x20)
        {
            begin = it;
            break;
        }

    for (auto it = string.rbegin(); it != string.rend(); ++it)
        if (*it > 0x20)
        {
            end = it.base();
            break;
        }

    return { std::make_move_iterator(begin), std::make_move_iterator(end) };
}

std::string unvm::Lower(std::string string)
{
    for (auto &it : string)
        it = static_cast<char>(std::tolower(it));
    return string;
}

std::vector<std::string> unvm::Split(const std::string &str, char delim)
{
    std::vector<std::string> vec;

    size_t beg = 0, end;
    for (; (end = str.find(delim, beg)) != std::string::npos; beg = end + 1)
    {
        if (beg == end)
            continue;
        vec.push_back(str.substr(beg, end - beg));
    }
    if (beg != end && beg < str.size())
        vec.push_back(str.substr(beg));

    return vec;
}

std::string unvm::Join(const std::vector<std::string> &vec, char delim)
{
    std::string str;
    
    for (auto it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin())
            str += delim;
        str += *it;
    }

    return str;
}
