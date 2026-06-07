#include <unvm/util.hxx>

std::istream &unvm::GetLine(std::istream &stream, std::string &string, const std::string_view delim)
{
    string.clear();

    while (stream.good() && !stream.eof() && string.find(delim) == std::string::npos)
    {
        const auto c = stream.get();
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
