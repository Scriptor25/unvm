#include <http.hxx>

#include <istream>
#include <ostream>

std::ostream &operator<<(std::ostream &stream, http::HttpMethod method)
{
    static const std::map<http::HttpMethod, const char *> map = {
        {http::HttpMethod::Get, "GET"},
        {http::HttpMethod::Head, "HEAD"},
        {http::HttpMethod::Post, "POST"},
        {http::HttpMethod::Put, "PUT"},
        {http::HttpMethod::Delete, "DELETE"},
        {http::HttpMethod::Connect, "CONNECT"},
        {http::HttpMethod::Options, "OPTIONS"},
        {http::HttpMethod::Trace, "TRACE"},
    };

    return stream << map.at(method);
}

std::istream &operator>>(std::istream &stream, http::HttpStatusCode &status_code)
{
    int status_code_int;
    stream >> status_code_int;
    status_code = static_cast<http::HttpStatusCode>(status_code_int);
    return stream;
}
