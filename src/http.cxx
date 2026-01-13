#include <http.hxx>

void http::ParseUrl(http::ResourceLocation &dst, const std::string &src)
{
    auto scheme_end = src.find("://");
    dst.Scheme = src.substr(0, scheme_end);

    auto host_begin = scheme_end + 3;
    auto path_begin = src.find('/', host_begin);

    auto host_port = path_begin == std::string::npos
                         ? src.substr(host_begin)
                         : src.substr(host_begin, path_begin - host_begin);

    dst.Path = path_begin == std::string::npos
                   ? "/"
                   : src.substr(path_begin);

    auto colon = host_port.find(':');
    if (colon != std::string::npos)
    {
        dst.Host = host_port.substr(0, colon);
        dst.Port = static_cast<std::uint16_t>(std::stoi(host_port.substr(colon + 1)));
    }
    else
    {
        dst.Host = host_port;
        dst.Port = dst.Scheme == "https" ? 443 : 80;
    }
}