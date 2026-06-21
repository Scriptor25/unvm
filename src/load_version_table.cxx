#include <unvm/json.hxx>
#include <unvm/unvm.hxx>
#include <unvm/util.hxx>
#include <unvm/http/url.hxx>

#include <fstream>
#include <sstream>

toolkit::result<> unvm::LoadVersionTable(http::HttpClient &client, VersionTable &table, bool online)
{
    /**
     * {
     *   version:  string
     *   date:     string
     *   files:    string[]
     *   npm?:     string
     *   v8:       string
     *   uv?:      string
     *   zlib?:    string
     *   openssl?: string
     *   modules?: string
     *   lts:      string | false
     *   security: boolean
     * }[]
     */

    auto data_directory = GetDataDirectory();
    auto index = data_directory / "index.json";

    bool is_stale{};
    if (online && std::filesystem::exists(index))
    {
        constexpr auto stale = std::chrono::hours(24);

        auto last_write = std::filesystem::last_write_time(index);
        auto now = std::filesystem::file_time_type::clock::now();

        is_stale = now - last_write > stale;
    }

    if ((online && is_stale) || !std::filesystem::exists(index))
    {
        std::stringstream stream;

        http::HttpRequest request
        {
            .Method = http::HttpMethod::Get,
            .Location = http::ParseURL("https://nodejs.org/dist/index.json"),
        };
        http::HttpResponse response
        {
            .Body = &stream,
        };

        if (auto res = client.FetchWithRedirects(std::move(request), response); !res)
        {
            return toolkit::make_error("failed to get file: {}", res.error());
        }

        if (!IsSuccess(response.StatusCode))
        {
            return toolkit::make_error(
                "failed to get file: {}, {}\n{}",
                response.StatusCode,
                response.StatusMessage,
                stream.str());
        }

        json::Node node;
        stream >> node;

        if (!(node >> table))
        {
            return toolkit::make_error("failed to parse table json.");
        }

        std::ofstream file(index);
        file << node;

        return {};
    }

    std::ifstream stream(index);

    json::Node node;
    stream >> node;

    if (!(node >> table))
    {
        return toolkit::make_error("failed to parse table json.");
    }

    return {};
}

void unvm::FilterVersionTable(const Config &config, VersionTable &table, const bool supported, const bool installed)
{
    const std::string pattern(platform.Pattern);

    for (auto it = table.begin(); it != table.end();)
    {
        const auto is_supported = it->Files.contains(pattern);
        const auto is_installed = config.Installed.contains(it->Version);

        if ((supported && !is_supported) || (installed && !is_installed))
        {
            it = table.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
