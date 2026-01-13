#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace http
{
    enum class Method
    {
        Get,
        Post,
        Put,
        Delete,
    };

    struct ResourceLocation
    {
        std::string Scheme;
        std::string Host;
        std::string Path;
        std::uint16_t Port;
    };

    using Headers = std::map<std::string, std::string>;

    class Client
    {
    public:
        Client();
        ~Client();

        int Request(
            Method method,
            const std::string &url,
            const Headers &headers,
            std::istream *src,
            std::ostream *dst);

    private:
        struct ClientState;
        ClientState *m_State;
    };

    void ParseUrl(ResourceLocation &dst, const std::string &src);
}
