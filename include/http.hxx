#pragma once

#include <cstdint>
#include <format>
#include <map>
#include <string>

namespace http
{
    constexpr auto EOL = "\r\n";
    constexpr auto EOL2 = "\r\n\r\n";

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
        Continue           = 100,
        SwitchingProtocols = 101,

        OK                          = 200,
        Created                     = 201,
        Accepted                    = 202,
        NonAuthoritativeInformation = 203,
        NoContent                   = 204,
        ResetConnection             = 205,
        PartialContent              = 206,

        MultipleChoices   = 300,
        MovedPermanently  = 301,
        Found             = 302,
        SeeOther          = 303,
        NotModified       = 304,
        UseProxy          = 305,
        TemporaryRedirect = 307,
        PermanentRedirect = 308,

        BadRequest                  = 400,
        Unauthorized                = 401,
        PaymentRequired             = 402,
        Forbidden                   = 403,
        NotFound                    = 404,
        MethodNotAllowed            = 405,
        NotAcceptable               = 406,
        ProxyAuthenticationRequired = 407,
        RequestTimeout              = 408,
        Conflict                    = 409,
        Gone                        = 410,
        LengthRequired              = 411,
        PreconditionFailed          = 412,
        ContentTooLarge             = 413,
        URITooLong                  = 414,
        UnsupportedMediaType        = 415,
        RangeNotSatisfiable         = 416,
        ExpectationFailed           = 417,
        MisdirectedRequest          = 421,
        UnprocessableContent        = 422,
        UpgradeRequired             = 426,

        InternalServerError     = 500,
        NotImplemented          = 501,
        BadGateway              = 502,
        ServiceUnavailable      = 503,
        GatewayTimeout          = 504,
        HTTPVersionNotSupported = 505,
    };

    inline bool is_directive(HttpStatusCode status_code)
    {
        return 100 <= static_cast<int>(status_code) && static_cast<int>(status_code) <= 199;
    }

    inline bool is_success(HttpStatusCode status_code)
    {
        return 200 <= static_cast<int>(status_code) && static_cast<int>(status_code) <= 299;
    }

    inline bool is_redirect(HttpStatusCode status_code)
    {
        return 300 <= static_cast<int>(status_code) && static_cast<int>(status_code) <= 399;
    }

    inline bool is_client_fail(HttpStatusCode status_code)
    {
        return 400 <= static_cast<int>(status_code) && static_cast<int>(status_code) <= 499;
    }

    inline bool is_server_fail(HttpStatusCode status_code)
    {
        return 500 <= static_cast<int>(status_code) && static_cast<int>(status_code) <= 599;
    }

    struct HttpLocation
    {
        std::string Scheme;
        std::string Host;
        std::uint16_t Port{};
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
        HttpStatusCode StatusCode;
        std::string StatusMessage;
        HttpHeaders Headers;
        std::ostream *Body;
    };

    int HttpParseStatus(std::istream &stream, HttpStatusCode &status_code, std::string &status_message);
    int HttpParseHeaders(std::istream &stream, HttpHeaders &headers);

    struct HttpTransport
    {
        virtual ~HttpTransport() = default;

        virtual int write(const char *buf, std::size_t len) = 0;
        virtual int read(char *buf, std::size_t len) = 0;
    };

    class HttpClient
    {
    public:
        HttpClient();
        ~HttpClient();

        int Request(HttpRequest request, HttpResponse &response);

    private:
        struct State;
        State *m_State{};
    };
}

std::ostream &operator<<(std::ostream &stream, http::HttpMethod method);
std::ostream &operator<<(std::ostream &stream, http::HttpStatusCode status_code);
std::istream &operator>>(std::istream &stream, http::HttpStatusCode &status_code);
std::ostream &operator<<(std::ostream &stream, http::HttpLocation &location);

template<>
struct std::formatter<http::HttpStatusCode> : std::formatter<int>
{
    template<typename Context>
    auto format(http::HttpStatusCode status_code, Context &context) const
    {
        return std::formatter<int>::format(static_cast<int>(status_code), context);
    }
};
