#include <unvm/util.hxx>
#include <unvm/http/http.hxx>
#include <unvm/http/url.hxx>

#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include <ssl/ca-bundle.h>

#include <charconv>
#include <iostream>
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

#if defined(SYSTEM_LINUX) || defined(SYSTEM_ANDROID) || defined(SYSTEM_DARWIN)

#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>

using platform_socket_t = int;

inline int socket_close(const platform_socket_t s)
{
    return close(s);
}

#endif

static int read_until(unvm::http::HttpTransport *transport, std::string &dst, const char *delim)
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

static void set_header_if_missing(unvm::http::HttpHeaders &headers, const std::string &key, const std::string &val)
{
    if (headers.contains(key) || headers.contains(unvm::Lower(key)))
    {
        return;
    }

    headers.emplace(key, val);
}

int unvm::http::ParseStatus(std::istream &stream, HttpStatusCode &status_code, std::string &status_message)
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

void unvm::http::ParseHeaders(std::istream &stream, HttpHeaders &headers)
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
}

struct HttpTcpTransport final : unvm::http::HttpTransport
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

struct HttpTlsTransport final : unvm::http::HttpTransport
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

struct unvm::http::HttpClient::State
{
#ifdef SYSTEM_WINDOWS
    WSADATA WsaData;
#endif
    SSL_CTX *SslCtx;
};

static int load_cert_chain_from_shared_mem(SSL_CTX *context, const void *buf, const int len)
{
    if (!context)
    {
        std::cerr << "ssl context is null." << std::endl;
        return 1;
    }

    if (!buf)
    {
        std::cerr << "buffer is null." << std::endl;
        return 1;
    }

    if (len <= 0)
    {
        std::cerr << "length is not greater than 0." << std::endl;
        return 1;
    }

    const auto bio = BIO_new_mem_buf(buf, len);
    if (!bio)
    {
        std::cerr << "failed to create bio." << std::endl;
        return 1;
    }

    const auto infos = PEM_X509_INFO_read_bio(bio, nullptr, nullptr, nullptr);
    if (!infos)
    {
        BIO_free(bio);

        std::cerr << "failed to read bio." << std::endl;
        return 1;
    }

    const auto store = SSL_CTX_get_cert_store(context);
    if (!store)
    {
        sk_X509_INFO_pop_free(infos, X509_INFO_free);
        BIO_free(bio);

        std::cerr << "failed to get certificate store for ssl context." << std::endl;
        return 1;
    }

    for (auto i = 0; i < sk_X509_INFO_num(infos); ++i)
    {
        if (const auto info = sk_X509_INFO_value(infos, i); info && info->x509)
        {
            if (!X509_STORE_add_cert(store, info->x509))
            {
                sk_X509_INFO_pop_free(infos, X509_INFO_free);
                BIO_free(bio);

                std::cerr << "failed to add certificate to store." << std::endl;
                return 1;
            }
        }
    }

    sk_X509_INFO_pop_free(infos, X509_INFO_free);
    BIO_free(bio);
    return 0;
}

unvm::http::HttpClient::HttpClient()
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

    SSL_CTX_set_verify(m_State->SslCtx, SSL_VERIFY_PEER, nullptr);

    if (const auto error = load_cert_chain_from_shared_mem(m_State->SslCtx, ca_bundle_data, static_cast<int>(ca_bundle_data_len)))
    {
        std::cerr << "failed to load vendor certificates." << std::endl;
        return;
    }
}

unvm::http::HttpClient::~HttpClient()
{
    SSL_CTX_free(m_State->SslCtx);
    ERR_free_strings();
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();

#ifdef SYSTEM_WINDOWS
    WSACleanup();
#endif

    delete m_State;
    m_State = nullptr;
}

int unvm::http::HttpClient::Fetch(HttpRequest request, HttpResponse &response)
{
    auto service = std::to_string(request.Location.Port);

    addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

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

        if (auto result = SSL_connect(ssl); result <= 0)
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

        while (true)
        {
            request.Body->read(buf, sizeof(buf));
            const auto len = request.Body->gcount();

            if (len <= 0)
            {
                break;
            }

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
    if (auto error = ParseStatus(status_stream, response.StatusCode, response.StatusMessage))
    {
        std::cerr << "failed to parse status line." << std::endl;
        return error;
    }

    ParseHeaders(headers_stream, response.Headers);

    std::size_t content_length = ~0ULL;
    if (auto it = response.Headers.find("content-length"); it != response.Headers.end())
    {
        auto [_, ec] = std::from_chars(it->second.data(), it->second.data() + it->second.size(), content_length);
        if (ec != std::errc())
        {
            std::cerr << "failed to parse content length header." << std::endl;
            return static_cast<int>(ec);
        }
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

    if (IsRedirect(response.StatusCode))
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
        return Fetch(std::move(request), response);
    }

    return 0;
}

std::ostream &operator<<(std::ostream &stream, const unvm::http::HttpMethod method)
{
    static const std::map<unvm::http::HttpMethod, const char *> map
    {
        { unvm::http::HttpMethod::Get, "GET" },
        { unvm::http::HttpMethod::Head, "HEAD" },
        { unvm::http::HttpMethod::Post, "POST" },
        { unvm::http::HttpMethod::Put, "PUT" },
        { unvm::http::HttpMethod::Delete, "DELETE" },
        { unvm::http::HttpMethod::Connect, "CONNECT" },
        { unvm::http::HttpMethod::Options, "OPTIONS" },
        { unvm::http::HttpMethod::Trace, "TRACE" },
    };

    return stream << map.at(method);
}

std::ostream &operator<<(std::ostream &stream, unvm::http::HttpStatusCode status_code)
{
    return stream << static_cast<int>(status_code);
}

std::istream &operator>>(std::istream &stream, unvm::http::HttpStatusCode &status_code)
{
    int status_code_int;
    stream >> status_code_int;
    status_code = static_cast<unvm::http::HttpStatusCode>(status_code_int);
    return stream;
}

std::ostream &operator<<(std::ostream &stream, const unvm::http::HttpLocation &location)
{
    return stream
           << (location.UseTLS ? "https" : "http")
           << "://"
           << location.Host
           << ":"
           << location.Port
           << location.Pathname;
}
