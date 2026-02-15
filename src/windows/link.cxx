#ifdef SYSTEM_WINDOWS

#include <util.hxx>

#include <filesystem>
#include <iostream>
#include <vector>

#include <windows.h>
#include <winioctl.h>

struct REPARSE_JUNCTION_DATA_BUFFER
{
    DWORD ReparseTag;
    WORD ReparseDataLength;
    WORD Reserved;

    WORD SubstituteNameOffset;
    WORD SubstituteNameLength;
    WORD PrintNameOffset;
    WORD PrintNameLength;
    WCHAR PathBuffer[1];
};

#ifndef REPARSE_DATA_BUFFER_HEADER_SIZE
#define REPARSE_DATA_BUFFER_HEADER_SIZE 8
#endif

static std::wstring GetErrorMessage(DWORD error)
{
    LPWSTR buffer = nullptr;

    DWORD len = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER
        | FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR) & buffer,
        0,
        nullptr);

    std::wstring result;
    if (len && buffer)
    {
        result.assign(buffer, len);
        LocalFree(buffer);
    }

    return result;
}

int CreateLink(const std::filesystem::path &link, const std::filesystem::path &target)
{
    if (!std::filesystem::exists(target) || !std::filesystem::is_directory(target))
    {
        return 1;
    }

    {
        std::error_code error;
        std::filesystem::create_directory_symlink(target, link, error);
        if (!error)
        {
            return 0;
        }
    }

    auto abs_link = std::filesystem::absolute(link).wstring();
    auto abs_target = std::filesystem::absolute(target).wstring();

    if (!CreateDirectoryW(abs_link.c_str(), nullptr))
    {
        if (auto error = GetLastError(); error != ERROR_ALREADY_EXISTS)
        {
            auto message = GetErrorMessage(error);
            std::wcerr << message << std::endl;
            return 1;
        }
    }

    HANDLE h = CreateFileW(
        abs_link.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ
        | FILE_SHARE_WRITE
        | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
        nullptr);

    if (h == INVALID_HANDLE_VALUE)
    {
        RemoveDirectoryW(abs_link.c_str());
        return 1;
    }

    auto nt_target = L"\\??\\" + abs_target;

    auto substitute_name_length = nt_target.size() * sizeof(WCHAR);
    auto print_name_length = abs_target.size() * sizeof(WCHAR);

    auto reparse_data_length = 4 * sizeof(WORD)
                               + substitute_name_length
                               + sizeof(WCHAR)
                               + print_name_length + sizeof(WCHAR);
    auto buffer_size = reparse_data_length
                       + sizeof(DWORD)
                       + 2 * sizeof(WORD);

    std::vector<BYTE> buffer(buffer_size);

    auto reparse = reinterpret_cast<REPARSE_JUNCTION_DATA_BUFFER *>(buffer.data());
    reparse->ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
    reparse->ReparseDataLength = reparse_data_length;
    reparse->Reserved = 0;
    reparse->SubstituteNameOffset = 0;
    reparse->SubstituteNameLength = substitute_name_length;
    reparse->PrintNameOffset = substitute_name_length + sizeof(WCHAR);
    reparse->PrintNameLength = print_name_length;

    memcpy(reparse->PathBuffer, nt_target.c_str(), substitute_name_length + sizeof(WCHAR));
    memcpy(
        (BYTE *) reparse->PathBuffer + reparse->PrintNameOffset,
        abs_target.c_str(),
        print_name_length + sizeof(WCHAR));

    DWORD bytes_returned;
    auto ok = DeviceIoControl(
        h,
        FSCTL_SET_REPARSE_POINT,
        reparse,
        reparse->ReparseDataLength + REPARSE_DATA_BUFFER_HEADER_SIZE,
        nullptr,
        0,
        &bytes_returned,
        nullptr);
    if (!ok)
    {
        auto error = GetLastError();
        auto message = GetErrorMessage(error);
        std::wcerr << message << std::endl;
    }

    CloseHandle(h);
    return ok ? 0 : 1;
}

int RemoveLink(const std::filesystem::path &link)
{
    if (std::filesystem::is_symlink(link))
    {
        std::error_code error;
        std::filesystem::remove(link, error);
        if (error)
        {
            std::cerr << "failed to remove link: " << error.message() << std::endl;
            return error.value();
        }
        return 0;
    }

    auto abs_link = std::filesystem::absolute(link).wstring();

    HANDLE h = CreateFileW(
        abs_link.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
        nullptr);

    if (h == INVALID_HANDLE_VALUE)
    {
        return 1;
    }

    REPARSE_JUNCTION_DATA_BUFFER reparse{};
    reparse.ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;

    DWORD bytes_returned;
    auto ok = DeviceIoControl(
        h,
        FSCTL_DELETE_REPARSE_POINT,
        &reparse,
        REPARSE_DATA_BUFFER_HEADER_SIZE,
        nullptr,
        0,
        &bytes_returned,
        nullptr);
    if (!ok)
    {
        auto error = GetLastError();
        auto message = GetErrorMessage(error);
        std::wcerr << message << std::endl;
    }

    CloseHandle(h);
    return ok ? 0 : 1;
}

#endif
