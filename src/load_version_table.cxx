#include <unvm/http.hxx>
#include <unvm/json.hxx>
#include <unvm/lock.hxx>
#include <unvm/unvm.hxx>
#include <unvm/util.hxx>

#include <fstream>
#include <sstream>

toolkit::result<> unvm::LoadVersionTable(const http::client &client, VersionTable &table, bool online)
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

    table.clear();

    auto data_directory = GetDataDirectory();

    if (std::error_code ec; std::filesystem::create_directories(data_directory, ec), ec)
    {
        return toolkit::make_error("failed to create data directory: {} ({}).", ec.message(), ec.value());
    }

    auto index_path = data_directory / "index.json";
    auto lock_path = data_directory / "index.lock";

    FileLock lock;
    if (auto res = FileLock::Lock(lock_path) >> lock; !res)
    {
        return res;
    }

    bool is_stale{};
    if (online && std::filesystem::exists(index_path))
    {
        constexpr auto stale = std::chrono::hours(1);

        auto last_write = std::filesystem::last_write_time(index_path);
        auto now = std::filesystem::file_time_type::clock::now();

        is_stale = now - last_write > stale;
    }

    if ((online && is_stale) || !std::filesystem::exists(index_path))
    {
        std::stringstream stream;

        http::request_t request
        {
            .method = http::method::get,
            .location = http::url::parse("https://nodejs.org/dist/index.json"),
        };
        http::response_t response
        {
            .body = &stream,
        };

        if (auto res = client.fetch_with_redirects(std::move(request), response); !res)
        {
            return toolkit::make_error("failed to get file: {}", res.error());
        }

        if (!http::is_success(response.code))
        {
            return toolkit::make_error(
                "failed to get file: {}, {}\n{}",
                response.code,
                response.message,
                stream.str());
        }

        json::node node;
        stream >> node;

        if (!(node >> table))
        {
            return toolkit::make_error("failed to parse table json.");
        }

        std::ofstream file(index_path);
        file << node;

        return {};
    }

    std::ifstream stream(index_path);

    json::node node;
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
