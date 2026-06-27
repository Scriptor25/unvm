#pragma once

#include <toolkit/result.hxx>

#include <filesystem>

namespace unvm
{
    class FileLock
    {
    public:
        [[nodiscard]] static toolkit::result<FileLock> Lock(const std::filesystem::path &path);

        FileLock() = default;
        ~FileLock();

        FileLock(const FileLock &) = delete;
        FileLock &operator=(const FileLock &) = delete;

        FileLock(FileLock &&other) noexcept;
        FileLock &operator=(FileLock &&other) noexcept;

    private:
#if defined(SYSTEM_WINDOWS)

        explicit FileLock(void *handle);

        void *m_Handle = nullptr;

#else

        explicit FileLock(int handle);

        int m_Handle = -1;

#endif
    };

    class TryAcquire
    {
    public:
        TryAcquire(const std::filesystem::path &path, bool wait, std::string_view message);
        ~TryAcquire();

        TryAcquire(const TryAcquire &) = delete;
        TryAcquire &operator=(const TryAcquire &) = delete;

        TryAcquire(TryAcquire &&other) noexcept;
        TryAcquire &operator=(TryAcquire &&other) noexcept;

        explicit operator bool() const;

        bool Primary() const;
        std::string_view Message() const;

    private:
        bool m_Acquired{}, m_Primary{};
        std::filesystem::path m_Path;
        std::string m_Message;
    };
}
