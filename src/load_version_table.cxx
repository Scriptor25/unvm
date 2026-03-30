#include <unvm/json.hxx>
#include <unvm/unvm.hxx>
#include <unvm/util.hxx>
#include <unvm/http/url.hxx>

#include <json/json.hxx>

#include <fstream>
#include <iostream>

int unvm::LoadVersionTable(http::Client &client, unvm::VersionTable &table, bool online)
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

    auto index = unvm::GetDataDirectory() / "index.json";

    if (online || !std::filesystem::exists(index))
    {
        std::stringstream stream;

        http::Request request = {
            .Method = http::Method::Get,
            .Location = http::ParseUrl("https://nodejs.org/dist/index.json"),
        };
        http::Response response = {
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

        json::Node json;
        stream >> json;
        json >> table;

        std::ofstream file(index);
        file << json;

        return 0;
    }

    std::ifstream stream(index);

    json::Node json;
    stream >> json;
    json >> table;

    return 0;
}
