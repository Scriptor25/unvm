#include <unvm/json.hxx>
#include <unvm/unvm.hxx>
#include <unvm/util.hxx>
#include <unvm/http/url.hxx>

#include <fstream>
#include <iostream>

int unvm::LoadVersionTable(http::HttpClient &client, VersionTable &table, bool online)
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

    auto index = GetDataDirectory() / "index.json";

    if (online || !std::filesystem::exists(index))
    {
        std::stringstream stream;

        http::HttpRequest request = {
            .Method = http::HttpMethod::Get,
            .Location = http::ParseUrl("https://nodejs.org/dist/index.json"),
        };
        http::HttpResponse response = {
            .Body = &stream,
        };

        if (auto error = client.Fetch(request, response))
        {
            std::cerr << "failed to get file." << std::endl;
            return error;
        }

        if (!http::IsSuccess(response.StatusCode))
        {
            std::cerr
                    << "failed to get file: "
                    << response.StatusCode
                    << ", "
                    << response.StatusMessage
                    << std::endl
                    << stream.str()
                    << std::endl;
            return 1;
        }

        json::Node node;
        stream >> node;

        if (!(node >> table))
        {
            std::cerr << "failed to parse table json." << std::endl;
            return 1;
        }

        std::ofstream file(index);
        file << node;

        return 0;
    }

    std::ifstream stream(index);

    json::Node node;
    stream >> node;

    if (!(node >> table))
    {
        std::cerr << "failed to parse table json." << std::endl;
        return 1;
    }

    return 0;
}
