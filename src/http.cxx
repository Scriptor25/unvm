#include <unvm/data.hxx>
#include <unvm/http.hxx>
#include <unvm/util.hxx>

#include <http/http.hxx>
#include <http/url.hxx>

#include <toolkit/defer.hxx>

#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include <iostream>
#include <memory>
#include <ostream>

#ifdef SYSTEM_WINDOWS

#include <winsock2.h>
#include <ws2tcpip.h>

inline int socket_close(int fd)
{
    return closesocket(fd);
}

#endif

#if defined(SYSTEM_LINUX) || defined(SYSTEM_ANDROID) || defined(SYSTEM_DARWIN)

#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>

inline int socket_close(const int fd)
{
    return close(fd);
}

#endif

[[nodiscard]] static toolkit::result<> load_vendor_certificates(
    const SSL_CTX *context,
    const std::span<const uint8_t> buffer)
{
    if (!context)
    {
        return toolkit::make_error("context is null.");
    }

    if (buffer.empty())
    {
        return toolkit::make_error("buffer is empty.");
    }

    const auto bio = BIO_new_mem_buf(buffer.data(), static_cast<int>(buffer.size()));
    if (!bio)
    {
        return toolkit::make_error("failed to create bio: {}", unvm::GetSSLErrorStack());
    }

    auto guard_bio = toolkit::defer(BIO_free, bio);

    const auto infos = PEM_X509_INFO_read_bio(bio, nullptr, nullptr, nullptr);
    if (!infos)
    {
        return toolkit::make_error("failed to read bio: {}", unvm::GetSSLErrorStack());
    }

    auto guard_infos = toolkit::defer(
        [](auto *i)
        {
            sk_X509_INFO_pop_free(i, X509_INFO_free);
        },
        infos);

    const auto store = SSL_CTX_get_cert_store(context);
    if (!store)
    {
        return toolkit::make_error("failed to get certificate store for context: {}", unvm::GetSSLErrorStack());
    }

    for (auto i = 0; i < sk_X509_INFO_num(infos); ++i)
    {
        if (const auto info = sk_X509_INFO_value(infos, i); info && info->x509)
        {
            if (!X509_STORE_add_cert(store, info->x509))
            {
                return toolkit::make_error("failed to add certificate to store: {}", unvm::GetSSLErrorStack());
            }
        }
    }

    return {};
}

struct PlatformTransport : http::Transport
{
    PlatformTransport()
    {
#ifdef SYSTEM_WINDOWS
        WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

        ctx = SSL_CTX_new(TLS_client_method());
        SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);

        if (auto res = load_vendor_certificates(ctx, unvm::data::cacert); !res)
        {
            std::cerr << "failed to load vendor certificates: " << res.error() << std::endl;
            return;
        }
    }

    ~PlatformTransport() override
    {
        SSL_CTX_free(ctx);

#ifdef SYSTEM_WINDOWS
        WSACleanup();
#endif
    }

    PlatformTransport(const PlatformTransport &) = delete;
    PlatformTransport &operator=(const PlatformTransport &) = delete;

    PlatformTransport(PlatformTransport &&other) noexcept
    {
#ifdef SYSTEM_WINDOWS
        std::swap(wsa, other.wsa);
#endif
        std::swap(ctx, other.ctx);
        std::swap(ssl, other.ssl);
    }

    PlatformTransport &operator=(PlatformTransport &&other) noexcept
    {
#ifdef SYSTEM_WINDOWS
        std::swap(wsa, other.wsa);
#endif
        std::swap(ctx, other.ctx);
        std::swap(ssl, other.ssl);

        return *this;
    }

    toolkit::result<int> open(const http::URL &location) override
    {
        if (location.Scheme != "http" && location.Scheme != "https")
        {
            return toolkit::make_error("unsupported scheme '{}'", location.Scheme);
        }

        auto service = std::to_string(location.Port);

        addrinfo hints
        {
            .ai_family = AF_UNSPEC,
            .ai_socktype = SOCK_STREAM,
            .ai_protocol = 0,
        };

        addrinfo *info{};
        if (auto error = getaddrinfo(location.Host.c_str(), service.c_str(), &hints, &info))
        {
            return toolkit::make_error("failed to get address info ({}).", error);
        }

        int fd = -1;

        for (auto it = info; it; it = it->ai_next)
        {
            fd = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
            if (fd < 0)
            {
                continue;
            }

            if (connect(fd, it->ai_addr, it->ai_addrlen))
            {
                socket_close(fd);
                fd = -1;
                continue;
            }

            break;
        }

        freeaddrinfo(info);

        if (fd < 0)
        {
            return toolkit::make_error("failed to open socket.");
        }

        auto *ssl_ = SSL_new(ctx);
        SSL_set_fd(ssl_, fd);

        SSL_set_tlsext_host_name(ssl_, location.Host.c_str());
        SSL_set1_host(ssl_, location.Host.c_str());
        SSL_set_verify(ssl_, SSL_VERIFY_PEER, nullptr);

        if (SSL_connect(ssl_) <= 0)
        {
            return toolkit::make_error("TLS handshake failed.");
        }

        if (SSL_get_verify_result(ssl_) != X509_V_OK)
        {
            return toolkit::make_error("TLS certificate verification failed.");
        }

        ssl[fd] = ssl_;
        return fd;
    }

    void close(int fd) override
    {
        SSL_free(ssl[fd]);
        socket_close(fd);
    }

    int recv(int fd, void *buffer, size_t count, int flags) override
    {
        return SSL_read(ssl[fd], buffer, static_cast<int>(count));
    }

    int send(int fd, const void *buffer, size_t count, int flags) override
    {
        return SSL_write(ssl[fd], buffer, static_cast<int>(count));
    }

#ifdef SYSTEM_WINDOWS
    WSADATA wsa{};
#endif
    SSL_CTX *ctx{};

    std::unordered_map<int, SSL *> ssl;
};

toolkit::result<std::unique_ptr<http::Transport>> unvm::CreateTransport()
{
    return { std::make_unique<PlatformTransport>() };
}
