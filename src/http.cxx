#include <http.hxx>
#include <iostream>
#include <url.hxx>
#include <util.hxx>

#include <openssl/err.h>
#include <openssl/ssl.h>

#include <istream>
#include <memory>
#include <ostream>
#include <sstream>

#ifdef SYSTEM_WINDOWS

#include <winsock2.h>
#include <ws2tcpip.h>

using platform_socket_t = SOCKET;

inline int socket_close(platform_socket_t s)
{
    return closesocket(s);
}

#endif

#if defined(SYSTEM_LINUX) || defined(SYSTEM_DARWIN)

#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>

using platform_socket_t = int;

inline int socket_close(const platform_socket_t s)
{
    return close(s);
}

#endif

static int read_until(http::HttpTransport *transport, std::string &dst, const char *delim)
{
    char buf[1024];

    while (dst.find(delim) == std::string::npos)
    {
        const auto len = transport->read(buf, sizeof(buf));
        if (len <= 0)
        {
            std::cerr << "failed to read chunk." << std::endl;
            return 1;
        }

        dst.append(buf, len);
    }

    return 0;
}

static void set_header_if_missing(http::HttpHeaders &headers, const std::string &key, const std::string &val)
{
    if (headers.contains(key) || headers.contains(Lower(key)))
    {
        return;
    }

    headers.emplace(key, val);
}

int http::HttpParseStatus(std::istream &stream, HttpStatusCode &status_code, std::string &status_message)
{
    std::string http_version; // TODO: check if version is supported
    stream >> http_version;

    if (http_version != "HTTP/1.1")
    {
        std::cerr << "invalid http version." << std::endl;
        return 1;
    }

    stream >> status_code;
    GetLine(stream, status_message, EOL);

    status_message = Trim(std::move(status_message));
    return 0;
}

int http::HttpParseHeaders(std::istream &stream, HttpHeaders &headers)
{
    headers.clear();

    std::string line;
    while (GetLine(stream, line, EOL))
    {
        if (line.empty())
        {
            break;
        }

        const auto colon = line.find(':');
        if (colon == std::string::npos)
        {
            continue;
        }

        auto key = line.substr(0, colon);
        auto val = line.substr(colon + 1);

        key = Trim(std::move(key));
        val = Trim(std::move(val));

        headers.emplace(Lower(std::move(key)), std::move(val));
    }
    return 0;
}

struct HttpTcpTransport final : http::HttpTransport
{
    explicit HttpTcpTransport(const platform_socket_t sock)
        : sock(sock)
    {
    }

    int write(const char *buf, const std::size_t len) override
    {
        return static_cast<int>(send(sock, buf, len, 0));
    }

    int read(char *buf, const std::size_t len) override
    {
        return static_cast<int>(recv(sock, buf, len, 0));
    }

    platform_socket_t sock;
};

struct HttpTlsTransport final : http::HttpTransport
{
    explicit HttpTlsTransport(SSL *ssl)
        : ssl(ssl)
    {
    }

    int write(const char *buf, const std::size_t len) override
    {
        return SSL_write(ssl, buf, static_cast<int>(len));
    }

    int read(char *buf, const std::size_t len) override
    {
        return SSL_read(ssl, buf, static_cast<int>(len));
    }

    SSL *ssl;
};

struct http::HttpClient::State
{
#ifdef SYSTEM_WINDOWS
    WSADATA WsaData;
#endif
    SSL_CTX *SslCtx;
};

http::HttpClient::HttpClient()
{
    m_State = new State();

#ifdef SYSTEM_WINDOWS
    WSAStartup(MAKEWORD(2, 2), &m_State->WsaData);
#endif

    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    m_State->SslCtx = SSL_CTX_new(TLS_client_method());
    SSL_CTX_set_min_proto_version(m_State->SslCtx, TLS1_2_VERSION);
    SSL_CTX_set_default_verify_paths(m_State->SslCtx);
    SSL_CTX_load_verify_file(m_State->SslCtx, "ssl/nodejs.pem");
}

http::HttpClient::~HttpClient()
{
    SSL_CTX_free(m_State->SslCtx);
    EVP_cleanup();

#ifdef SYSTEM_WINDOWS
    WSACleanup();
#endif

    delete m_State;
    m_State = nullptr;
}

int http::HttpClient::Request(HttpRequest request, HttpResponse &response)
{
    auto service = std::to_string(request.Location.Port);

    addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(request.Location.Host.c_str(), service.c_str(), &hints, &res))
    {
        std::cerr << "failed to get address info." << std::endl;
        return 1;
    }

    platform_socket_t sock = -1;

    for (auto it = res; it; it = it->ai_next)
    {
        sock = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (sock < 0)
        {
            continue;
        }

        if (connect(sock, it->ai_addr, it->ai_addrlen))
        {
            socket_close(sock);
            sock = -1;
            continue;
        }

        break;
    }

    freeaddrinfo(res);

    if (sock < 0)
    {
        std::cerr << "failed to open socket." << std::endl;
        return 1;
    }

    std::unique_ptr<HttpTransport> transport;
    SSL *ssl = nullptr;

    if (request.Location.UseTLS)
    {
        ssl = SSL_new(m_State->SslCtx);
        SSL_set_fd(ssl, sock);

        SSL_set_tlsext_host_name(ssl, request.Location.Host.c_str());
        SSL_set1_host(ssl, request.Location.Host.c_str());
        SSL_set_verify(ssl, SSL_VERIFY_PEER, nullptr);

        if (SSL_connect(ssl) <= 0)
        {
            SSL_free(ssl);

            socket_close(sock);
            std::cerr << "TLS handshake failed." << std::endl;
            return 1;
        }

        if (SSL_get_verify_result(ssl) != X509_V_OK)
        {
            SSL_free(ssl);

            socket_close(sock);
            std::cerr << "TLS certificate verification failed." << std::endl;
            return 1;
        }

        transport = std::make_unique<HttpTlsTransport>(ssl);
    }
    else
    {
        transport = std::make_unique<HttpTcpTransport>(sock);
    }

    set_header_if_missing(request.Headers, "Host", request.Location.Host);
    set_header_if_missing(request.Headers, "Connection", "close");
    set_header_if_missing(request.Headers, "Accept-Encoding", "identity");
    set_header_if_missing(request.Headers, "User-Agent", "unvm/0.1");

    std::stringstream packet;
    packet << request.Method << ' ' << request.Location.Pathname << ' ' << "HTTP/1.1" << EOL;
    for (auto &[key, val] : request.Headers)
    {
        packet << key << ": " << val << EOL;
    }
    packet << EOL;

    if (transport->write(packet.str().data(), packet.str().size()) < 0)
    {
        if (ssl)
        {
            SSL_free(ssl);
        }

        socket_close(sock);
        std::cerr << "failed to send header." << std::endl;
        return 1;
    }

    char buf[4096];

    if (request.Body)
    {
        std::size_t count = 0;

        while (request.Body->good())
        {
            auto len = request.Body->readsome(buf, sizeof(buf));

            if (transport->write(buf, len) < 0)
            {
                if (ssl)
                {
                    SSL_free(ssl);
                }

                socket_close(sock);
                std::cerr << "failed to send chunk." << std::endl;
                return 1;
            }

            count += len;
        }
    }

    std::string header_block;
    if (auto error = read_until(transport.get(), header_block, EOL2))
    {
        if (ssl)
        {
            SSL_free(ssl);
        }

        socket_close(sock);
        std::cerr << "failed to read header block." << std::endl;
        return error;
    }

    auto headers_end = header_block.find(EOL2);
    auto headers = header_block.substr(0, headers_end);
    auto body_prefetch = header_block.substr(headers_end + 4);

    std::istringstream headers_stream(headers);

    std::string status_line;
    GetLine(headers_stream, status_line, EOL);

    std::istringstream status_stream(status_line);
    if (auto error = HttpParseStatus(status_stream, response.StatusCode, response.StatusMessage))
    {
        std::cerr << "failed to parse status line." << std::endl;
        return error;
    }

    if (auto error = HttpParseHeaders(headers_stream, response.Headers))
    {
        std::cerr << "failed to parse headers." << std::endl;
        return error;
    }

    std::size_t content_length = ~0ULL;
    if (auto it = response.Headers.find("content-length"); it != response.Headers.end())
    {
        content_length = std::stoull(it->second);
    }

    if (response.Body && response.Body->good())
    {
        response.Body->write(body_prefetch.data(), static_cast<long>(body_prefetch.size()));
    }

    auto count = body_prefetch.size();
    while (content_length == ~0ULL || count < content_length)
    {
        auto len = transport->read(buf, sizeof(buf));
        if (len <= 0)
        {
            break;
        }

        if (response.Body && response.Body->good())
        {
            response.Body->write(buf, len);
        }

        count += len;
    }

    if (ssl)
    {
        SSL_free(ssl);
    }

    socket_close(sock);

    if (is_redirect(response.StatusCode))
    {
        if (!response.Headers.contains("location"))
        {
            std::cerr << response.Headers << std::endl;
            std::cerr << "missing location header in redirect response." << std::endl;
            return 1;
        }

        auto location = response.Headers.at("location");

        if (location.find("://") != std::string::npos)
        {
            request.Location = ParseUrl(location);
        }
        else if (location.starts_with("/"))
        {
            request.Location.Pathname = location;
        }
        else
        {
            request.Location.Pathname += location;
        }

        std::cerr << "redirect to " << location << " --> " << request.Location << std::endl;
        return Request(std::move(request), response);
    }

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

std::ostream &operator<<(std::ostream &stream, const http::HttpLocation &location)
{
    return stream
           << (location.UseTLS ? "https" : "http")
           << "://"
           << location.Host
           << ":"
           << location.Port
           << location.Pathname;
}
