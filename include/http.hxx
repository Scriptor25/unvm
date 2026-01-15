#pragma once

#include <cstdint>
#include <format>
#include <map>
#include <string>
#include <vector>

namespace http
{
    enum class HttpMethod
    {
        Get,
        Head,
        Post,
        Put,
        Delete,
        Connect,
        Options,
        Trace,
    };

    enum class HttpStatusCode : int
    {
        Continue = 100,
        SwitchingProtocols = 101,

        OK = 200,
        Created = 201,
        Accepted = 202,
        NonAuthoritativeInformation = 203,
        NoContent = 204,
        ResetConnection = 205,
        PartialContent = 206,

        MultipleChoices = 300,
        MovedPermanently = 301,
        Found = 302,
        SeeOther = 303,
        NotModified = 304,
        UseProxy = 305,
        TemporaryRedirect = 307,
        PermanentRedirect = 308,

        BadRequest = 400,
        Unauthorized = 401,
        PaymentRequired = 402,
        Forbidden = 403,
        NotFound = 404,
        MethodNotAllowed = 405,
        NotAcceptable = 406,
        ProxyAuthenticationRequired = 407,
        RequestTimeout = 408,
        Conflict = 409,
        Gone = 410,
        LengthRequired = 411,
        PreconditionFailed = 412,
        ContentTooLarge = 413,
        URITooLong = 414,
        UnsupportedMediaType = 415,
        RangeNotSatisfiable = 416,
        ExpectationFailed = 417,
        MisdirectedRequest = 421,
        UnprocessableContent = 422,
        UpgradeRequired = 426,

        InternalServerError = 500,
        NotImplemented = 501,
        BadGateway = 502,
        ServiceUnavailable = 503,
        GatewayTimeout = 504,
        HTTPVersionNotSupported = 505,
    };

    inline bool is_directive(HttpStatusCode status_code) { return 100 <= static_cast<int>(status_code) && static_cast<int>(status_code) <= 199; }
    inline bool is_success(HttpStatusCode status_code) { return 200 <= static_cast<int>(status_code) && static_cast<int>(status_code) <= 299; }
    inline bool is_redirect(HttpStatusCode status_code) { return 300 <= static_cast<int>(status_code) && static_cast<int>(status_code) <= 399; }
    inline bool is_client_fail(HttpStatusCode status_code) { return 400 <= static_cast<int>(status_code) && static_cast<int>(status_code) <= 499; }
    inline bool is_server_fail(HttpStatusCode status_code) { return 500 <= static_cast<int>(status_code) && static_cast<int>(status_code) <= 599; }

    struct HttpLocation
    {
        std::string Scheme;
        std::string Host;
        std::uint16_t Port;
        std::string Pathname;
    };

    using HttpHeaders = std::map<std::string, std::string>;

    struct HttpRequest
    {
        HttpMethod Method;
        HttpLocation Location;
        HttpHeaders Headers;
        std::istream *Body;
    };

    struct HttpResponse
    {
        inline bool ok() const { return is_success(StatusCode); }

        HttpStatusCode StatusCode;
        std::string StatusMessage;
        HttpHeaders Headers;
        std::ostream *Body;
    };

    class HttpClient
    {
    public:
        HttpClient();
        ~HttpClient();

        bool Request(HttpRequest request, HttpResponse &response);

    private:
        struct State;
        State *m_State;
    };

    constexpr void ParseUrl(HttpLocation &dst, const std::string &src)
    {
        auto scheme_end = src.find("://");
        dst.Scheme = src.substr(0, scheme_end);

        auto host_begin = scheme_end + 3;
        auto path_begin = src.find('/', host_begin);

        auto host_port = path_begin == std::string::npos
                             ? src.substr(host_begin)
                             : src.substr(host_begin, path_begin - host_begin);

        dst.Pathname = path_begin == std::string::npos
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

    constexpr HttpLocation ParseUrl(const std::string &src)
    {
        HttpLocation dst;
        ParseUrl(dst, src);
        return dst;
    }
}

std::ostream &operator<<(std::ostream &stream, http::HttpMethod method);
std::istream &operator>>(std::istream &stream, http::HttpStatusCode &status_code);

template <>
struct std::formatter<http::HttpStatusCode> : std::formatter<int>
{
    template <typename Context>
    auto format(http::HttpStatusCode status_code, Context &context) const
    {
        return std::formatter<int>::format(static_cast<int>(status_code), context);
    }
};
