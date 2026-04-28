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

    if (online || !exists(index))
    {
        std::stringstream stream;

        http::HttpRequest request
        {
            .Method = http::HttpMethod::Get,
            .Location = http::ParseUrl("https://nodejs.org/dist/index.json"),
        };
        http::HttpResponse response
        {
            .Body = &stream,
        };

        if (auto res = client.Fetch(std::move(request), response); !res)
        {
            return toolkit::make_error("failed to get file: {}", res.error());
        }

        if (!IsSuccess(response.StatusCode))
        {
            return toolkit::make_error("failed to get file: {}, {}\n{}", response.StatusCode, response.StatusMessage, stream.str());
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
