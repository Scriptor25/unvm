#include <fstream>
#include <unvm/lock.hxx>

#include <thread>

#if defined(SYSTEM_WINDOWS)

#define NOMINMAX

#include <windows.h>

#else

#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>

#endif

toolkit::result<unvm::FileLock> unvm::FileLock::Lock(const std::filesystem::path &path)
{
    const auto path_string = path.string();

#if defined(SYSTEM_WINDOWS)

    auto handle = CreateFileA(
        path_string.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (handle == INVALID_HANDLE_VALUE)
    {
        return toolkit::make_error("failed to open lock file.");
    }

    OVERLAPPED overlapped{};

    if (!LockFileEx(handle, LOCKFILE_EXCLUSIVE_LOCK, 0, MAXDWORD, MAXDWORD, &overlapped))
    {
        CloseHandle(handle);
        return toolkit::make_error("failed to acquire lock.");
    }

    return FileLock(handle);

#else

    const auto fd = open(path_string.c_str(), O_CREAT | O_RDWR, 0666);

    if (fd < 0)
    {
        return toolkit::make_error("failed to open lock file.");
    }

    if (flock(fd, LOCK_EX) != 0)
    {
        close(fd);
        return toolkit::make_error("failed to acquire lock.");
    }

    return FileLock(fd);

#endif
}

unvm::FileLock::~FileLock()
{
#if defined(SYSTEM_WINDOWS)

    if (m_Handle)
    {
        OVERLAPPED overlapped{};
        UnlockFileEx(static_cast<HANDLE>(m_Handle), 0, MAXDWORD, MAXDWORD, &overlapped);
        CloseHandle(static_cast<HANDLE>(m_Handle));
    }

#else

    if (m_Handle >= 0)
    {
        flock(m_Handle, LOCK_UN);
        close(m_Handle);
    }

#endif
}

unvm::FileLock::FileLock(FileLock &&other) noexcept
{
    std::swap(m_Handle, other.m_Handle);
}

unvm::FileLock &unvm::FileLock::operator=(FileLock &&other) noexcept
{
    std::swap(m_Handle, other.m_Handle);
    return *this;
}

#if defined(SYSTEM_WINDOWS)

unvm::FileLock::FileLock(void *handle)
    : m_Handle(handle)
{
}

#else

unvm::FileLock::FileLock(const int handle)
    : m_Handle(handle)
{
}

#endif

static bool try_acquire(const std::filesystem::path &path, std::string_view message)
{
    const auto path_string = path.string();

#if defined(SYSTEM_WINDOWS)

    auto handle = CreateFileA(
        path_string.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (handle == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    WriteFile(handle, message.data(), message.size(), nullptr, nullptr);
    CloseHandle(handle);

#else

    const auto fd = open(path_string.c_str(), O_CREAT | O_EXCL | O_WRONLY, 0666);

    if (fd < 0)
    {
        return false;
    }

    write(fd, message.data(), message.size());
    close(fd);

#endif

    return true;
}

unvm::TryAcquire::TryAcquire(const std::filesystem::path &path, const bool wait, const std::string_view message)
    : m_Path(path)
{
    if (try_acquire(path, message))
    {
        m_Primary = true;
        m_Acquired = true;
        m_Message = message;
        return;
    }

    if (!wait)
    {
        std::ifstream stream(path);
        std::getline(stream, m_Message);
        return;
    }

    auto delay = 50;
    for (;;)
    {
        if (try_acquire(path, message))
        {
            m_Acquired = true;
            return;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(delay));

        delay = std::min(delay * 2, 1000);
    }
}

unvm::TryAcquire::~TryAcquire()
{
    if (m_Acquired)
    {
        m_Acquired = false;
        std::filesystem::remove(m_Path);
    }
}

unvm::TryAcquire::TryAcquire(TryAcquire &&other) noexcept
{
    std::swap(m_Acquired, other.m_Acquired);
    std::swap(m_Path, other.m_Path);
}

unvm::TryAcquire &unvm::TryAcquire::operator=(TryAcquire &&other) noexcept
{
    std::swap(m_Acquired, other.m_Acquired);
    std::swap(m_Path, other.m_Path);
    return *this;
}

unvm::TryAcquire::operator bool() const
{
    return m_Acquired;
}

bool unvm::TryAcquire::Primary() const
{
    return m_Primary;
}

std::string_view unvm::TryAcquire::Message() const
{
    return m_Message;
}
