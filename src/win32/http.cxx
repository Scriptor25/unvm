#ifdef _WIN32

#include <http.hxx>

#include <istream>
#include <ostream>
#include <iostream>
#include <sstream>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

constexpr auto EOL = "\r\n";
constexpr auto EOL2 = "\r\n\r\n";

static bool recv_until(SOCKET sock, std::string &dst, const char *delim)
{
    char buf[1024];

    while (dst.find(delim) == std::string::npos)
    {
        auto len = recv(sock, buf, sizeof(buf), 0);
        if (len <= 0)
            return false;

        dst.append(buf, len);
    }

    return true;
}

static std::istream &getline(std::istream &stream, std::string &str, const std::string_view &delim)
{
    str.clear();

    while (stream.good() && str.find(delim) == std::string::npos)
        str += stream.get();

    if (str.find(delim) == std::string::npos)
        return stream;

    str = str.substr(0, str.size() - delim.size());
    return stream;
}

static void trim(std::string &str)
{
    if (str.empty())
        return;

    for (auto it = str.begin(); it != str.end(); ++it)
    {
        if (!std::isspace(*it))
            break;

        str.erase(it);
    }

    for (auto it = str.rbegin(); it != str.rend(); ++it)
    {
        if (!std::isspace(*it))
            break;

        str.erase(it.base());
    }
}

static std::string tolower(std::string str)
{
    for (auto it = str.begin(); it != str.end(); ++it)
        *it = std::tolower(*it);
    return str;
}

static void parse_status(std::istream &stream, http::HttpStatusCode &status_code, std::string &status_message)
{
    std::string http_version;

    stream >> http_version >> status_code;
    std::getline(stream, status_message);

    trim(status_message);
}

static void parse_headers(std::istream &stream, http::HttpHeaders &headers)
{
    std::string line;
    while (getline(stream, line, EOL))
    {
        if (line.empty())
            break;

        auto colon = line.find(':');
        if (colon == std::string::npos)
            continue;

        auto key = line.substr(0, colon);
        auto val = line.substr(colon + 1);

        trim(key);
        trim(val);

        headers.emplace(tolower(std::move(key)), std::move(val));
    }
}

static void set_header_if_missing(http::HttpHeaders &headers, const std::string &key, const std::string &val)
{
    if (headers.contains(key) || headers.contains(tolower(key)))
        return;

    headers.emplace(key, val);
}

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

bool http::HttpClient::Request(HttpRequest request, HttpResponse &response)
{
    auto service = std::to_string(request.Location.Port);

    addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(request.Location.Host.c_str(), service.c_str(), &hints, &res))
        return false;

    SOCKET sock = INVALID_SOCKET;

    for (auto it = res; it; it = it->ai_next)
    {
        sock = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (sock == INVALID_SOCKET)
            continue;

        if (connect(sock, it->ai_addr, it->ai_addrlen))
        {
            closesocket(sock);
            sock = INVALID_SOCKET;
            continue;
        }

        break;
    }

    freeaddrinfo(res);

    if (sock == INVALID_SOCKET)
        return false;

    set_header_if_missing(request.Headers, "Host", request.Location.Host);
    set_header_if_missing(request.Headers, "Connection", "close");
    set_header_if_missing(request.Headers, "Accept-Encoding", "identity");
    set_header_if_missing(request.Headers, "User-Agent", "unvm/0.1");

    std::stringstream packet;
    packet << request.Method << ' ' << request.Location.Pathname << ' ' << "HTTP/1.1" << EOL;
    for (auto &[key, val] : request.Headers)
        packet << key << ": " << val << EOL;
    packet << EOL;

    if (send(sock, packet.str().data(), packet.str().size(), 0) == SOCKET_ERROR)
    {
        closesocket(sock);
        return false;
    }

    char buf[1024];

    if (request.Body)
    {
        while (request.Body->good())
        {
            auto len = request.Body->readsome(buf, sizeof(buf));

            if (send(sock, buf, len, 0) == SOCKET_ERROR)
            {
                closesocket(sock);
                return false;
            }
        }
    }

    std::string header_block;
    if (!recv_until(sock, header_block, EOL2))
    {
        closesocket(sock);
        return false;
    }

    auto headers_end = header_block.find(EOL2);
    auto headers = header_block.substr(0, headers_end);
    auto body_prefetch = header_block.substr(headers_end + 4);

    std::istringstream headers_stream(headers);

    std::string status_line;
    getline(headers_stream, status_line, EOL);
    std::istringstream status_stream(status_line);

    parse_status(status_stream, response.StatusCode, response.StatusMessage);
    parse_headers(headers_stream, response.Headers);

    std::size_t content_length = ~0ULL;
    if (auto it = response.Headers.find("content-length"); it != response.Headers.end())
        content_length = std::stoull(it->second);

    if (response.Body && response.Body->good())
        response.Body->write(body_prefetch.data(), body_prefetch.size());

    if (content_length == ~0ULL)
    {
        while (true)
        {
            auto len = recv(sock, buf, sizeof(buf), 0);
            if (len <= 0)
                break;

            if (response.Body && response.Body->good())
                response.Body->write(buf, len);
        }
    }
    else
    {
        auto written = body_prefetch.size();

        while (written < content_length)
        {
            auto len = recv(sock, buf, sizeof(buf), 0);
            if (len <= 0)
                break;

            if (response.Body && response.Body->good())
                response.Body->write(buf, len);

            written += len;
        }
    }

    closesocket(sock);
    return true;
}

#endif