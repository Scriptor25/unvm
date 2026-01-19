#include <http.hxx>
#include <iostream>
#include <util.hxx>

#include <istream>
#include <ostream>

#ifdef SYSTEM_WINDOWS

#include <winsock2.h>
#include <ws2tcpip.h>

using platform_socket_t = SOCKET;

constexpr platform_socket_t PLATFORM_INVALID_SOCKET = INVALID_SOCKET;
constexpr int PLATFORM_SOCKET_ERROR = SOCKET_ERROR;

inline int socket_close(platform_socket_t s)
{
    return closesocket(s);
}

#endif

#ifdef SYSTEM_LINUX

#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>

using platform_socket_t = int;

constexpr platform_socket_t PLATFORM_INVALID_SOCKET = -1;
constexpr int PLATFORM_SOCKET_ERROR = -1;

inline int socket_close(platform_socket_t s)
{
    return close(s);
}

#endif

static int recv_until(platform_socket_t sock, std::string &dst, const char *delim)
{
    char buf[1024];

    while (dst.find(delim) == std::string::npos)
    {
        const auto len = recv(sock, buf, sizeof(buf), 0);
        if (len <= 0)
            return 1;

        dst.append(buf, len);
    }

    return 0;
}

static void set_header_if_missing(http::HttpHeaders &headers, const std::string &key, const std::string &val)
{
    if (headers.contains(key) || headers.contains(Lower(key)))
        return;

    headers.emplace(key, val);
}

int http::HttpParseStatus(std::istream &stream, HttpStatusCode &status_code, std::string &status_message)
{
    std::string http_version;

    stream >> http_version >> status_code;
    GetLine(stream, status_message, EOL);

    status_message = Trim(std::move(status_message));
    return 0;
}

int http::HttpParseHeaders(std::istream &stream, HttpHeaders &headers)
{
    std::string line;
    while (GetLine(stream, line, EOL))
    {
        if (line.empty())
            break;

        const auto colon = line.find(':');
        if (colon == std::string::npos)
            continue;

        auto key = line.substr(0, colon);
        auto val = line.substr(colon + 1);

        key = Trim(std::move(key));
        val = Trim(std::move(val));

        headers.emplace(Lower(std::move(key)), std::move(val));
    }
    return 0;
}

#ifdef SYSTEM_WINDOWS

struct http::HttpClient::State
{
    WSADATA WsaData;
};

http::HttpClient::HttpClient()
{
    m_State = new State();
    WSAStartup(MAKEWORD(2, 2), &m_State->WsaData);
}

http::HttpClient::~HttpClient()
{
    WSACleanup();
    delete m_State;
    m_State = nullptr;
}

#endif

#ifdef SYSTEM_LINUX

http::HttpClient::HttpClient() = default;
http::HttpClient::~HttpClient() = default;

#endif

int http::HttpClient::Request(HttpRequest request, HttpResponse &response)
{
    auto service = std::to_string(request.Location.Port);

    addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(request.Location.Host.c_str(), service.c_str(), &hints, &res))
        return 1;

    platform_socket_t sock = PLATFORM_INVALID_SOCKET;

    for (auto it = res; it; it = it->ai_next)
    {
        sock = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (sock == PLATFORM_INVALID_SOCKET)
            continue;

        if (connect(sock, it->ai_addr, it->ai_addrlen))
        {
            socket_close(sock);
            sock = PLATFORM_INVALID_SOCKET;
            continue;
        }

        break;
    }

    freeaddrinfo(res);

    if (sock == PLATFORM_INVALID_SOCKET)
        return 1;

    set_header_if_missing(request.Headers, "Host", request.Location.Host);
    set_header_if_missing(request.Headers, "Connection", "close");
    set_header_if_missing(request.Headers, "Accept-Encoding", "identity");
    set_header_if_missing(request.Headers, "User-Agent", "unvm/0.1");

    std::stringstream packet;
    packet << request.Method << ' ' << request.Location.Pathname << ' ' << "HTTP/1.1" << EOL;
    for (auto &[key, val] : request.Headers)
        packet << key << ": " << val << EOL;
    packet << EOL;

    if (send(sock, packet.str().data(), packet.str().size(), 0) == PLATFORM_SOCKET_ERROR)
    {
        socket_close(sock);
        return 1;
    }

    char buf[4096];

    if (request.Body)
    {
        std::size_t count = 0;

        while (request.Body->good())
        {
            auto len = request.Body->readsome(buf, sizeof(buf));

            if (send(sock, buf, len, 0) == PLATFORM_SOCKET_ERROR)
            {
                socket_close(sock);
                return 1;
            }

            count += len;
        }
    }

    std::string header_block;
    if (auto error = recv_until(sock, header_block, EOL2))
    {
        socket_close(sock);
        return error;
    }

    auto headers_end = header_block.find(EOL2);
    auto headers = header_block.substr(0, headers_end);
    auto body_prefetch = header_block.substr(headers_end + 4);

    std::istringstream headers_stream(headers);

    std::string status_line;
    GetLine(headers_stream, status_line, EOL);

    std::istringstream status_stream(status_line);
    HttpParseStatus(status_stream, response.StatusCode, response.StatusMessage);

    HttpParseHeaders(headers_stream, response.Headers);

    std::size_t content_length = ~0ULL;
    if (auto it = response.Headers.find("content-length"); it != response.Headers.end())
        content_length = std::stoull(it->second);

    if (response.Body && response.Body->good())
        response.Body->write(body_prefetch.data(), static_cast<long>(body_prefetch.size()));

    auto count = body_prefetch.size();
    while (content_length == ~0ULL || count < content_length)
    {
        auto len = recv(sock, buf, sizeof(buf), 0);
        if (len <= 0)
            break;

        if (response.Body && response.Body->good())
            response.Body->write(buf, len);

        count += len;
    }

    socket_close(sock);
    return 0;
}

std::ostream &operator<<(std::ostream &stream, const http::HttpMethod method)
{
    static const std::map<http::HttpMethod, const char *> map = {
        { http::HttpMethod::Get, "GET" },
        { http::HttpMethod::Head, "HEAD" },
        { http::HttpMethod::Post, "POST" },
        { http::HttpMethod::Put, "PUT" },
        { http::HttpMethod::Delete, "DELETE" },
        { http::HttpMethod::Connect, "CONNECT" },
        { http::HttpMethod::Options, "OPTIONS" },
        { http::HttpMethod::Trace, "TRACE" },
    };

    return stream << map.at(method);
}

std::ostream &operator<<(std::ostream &stream, http::HttpStatusCode status_code)
{
    return stream << static_cast<int>(status_code);
}

std::istream &operator>>(std::istream &stream, http::HttpStatusCode &status_code)
{
    int status_code_int;
    stream >> status_code_int;
    status_code = static_cast<http::HttpStatusCode>(status_code_int);
    return stream;
}
