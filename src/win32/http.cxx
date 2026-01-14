#ifdef _WIN32

#include <http.hxx>

#include <istream>
#include <ostream>
#include <iostream>

#include <windows.h>
#include <winhttp.h>

struct http::Client::ClientState
{
    HINTERNET Session;
};

http::Client::Client()
{
    m_State = new ClientState();
    m_State->Session = WinHttpOpen(L"unvm/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
}

http::Client::~Client()
{
    WinHttpCloseHandle(m_State->Session);
    delete m_State;
}

static const wchar_t *http_method(http::Method method)
{
    switch (method)
    {
    case http::Method::Get:
        return L"GET";
    case http::Method::Post:
        return L"POST";
    case http::Method::Put:
        return L"PUT";
    case http::Method::Delete:
        return L"DELETE";
    }

    return L"GET";
}

int http::Client::Request(
    Method method,
    const std::string &url,
    const Headers &headers,
    std::istream *src,
    std::ostream *dst)
{
    int status;

    ResourceLocation location;
    ParseUrl(location, url);

    std::wstring host(location.Host.begin(), location.Host.end());
    std::wstring path(location.Path.begin(), location.Path.end());

    HINTERNET connection = WinHttpConnect(
        m_State->Session,
        host.c_str(),
        location.Port,
        0);

    DWORD flags = 0;
    if (location.Scheme == "https")
        flags |= WINHTTP_FLAG_SECURE;

    LPCWSTR accept_types[]{
        L"*/*",
        nullptr,
    };

    HINTERNET request = WinHttpOpenRequest(
        connection,
        http_method(method),
        path.c_str(),
        nullptr,
        WINHTTP_NO_REFERER,
        accept_types,
        flags);

    for (auto &[key, val] : headers)
    {
        auto line = std::wstring(key.begin(), key.end()) + L": " + std::wstring(val.begin(), val.end());

        WinHttpAddRequestHeaders(
            request,
            line.c_str(),
            -1,
            WINHTTP_ADDREQ_FLAG_ADD);
    }

    WinHttpSendRequest(
        request,
        WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        WINHTTP_NO_REQUEST_DATA,
        0,
        WINHTTP_IGNORE_REQUEST_TOTAL_LENGTH,
        0);

    if (src)
    {
        char chunk[512];
        DWORD bytes_written;

        while (src->good())
        {
            src->read(chunk, sizeof(chunk));
            auto bytes_read = src->gcount();

            if (!bytes_read)
                break;

            WinHttpWriteData(request, chunk, bytes_read, &bytes_written);
        }
    }

    WinHttpReceiveResponse(request, nullptr);

    DWORD status_size = sizeof(DWORD);
    WinHttpQueryHeaders(
        request,
        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        nullptr,
        &status,
        &status_size,
        nullptr);

    if (dst)
    {
        char chunk[512];
        DWORD bytes_available, bytes_read;
        while (WinHttpQueryDataAvailable(request, &bytes_available) && bytes_available)
        {
            WinHttpReadData(
                request,
                chunk,
                std::min<DWORD>(bytes_available, sizeof(chunk)),
                &bytes_read);

            dst->write(chunk, bytes_read);
        }

        dst->flush();
    }

    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connection);

    return status;
}

#endif