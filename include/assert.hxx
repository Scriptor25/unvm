#pragma once

#include <format>
#include <iostream>

template <typename... Args>
[[noreturn]] void Error(std::format_string<Args...> format, Args &&...args)
{
    auto message = std::format(std::move(format), std::forward<Args>(args)...);
    std::cerr << message << std::endl;

    exit(1);
}

template <typename... Args>
[[noreturn]] void Error(std::wformat_string<Args...> format, Args &&...args)
{
    auto message = std::format(std::move(format), std::forward<Args>(args)...);
    std::wcerr << message << std::endl;

    exit(1);
}

template <typename... Args>
void Assert(bool exp, std::format_string<Args...> format, Args &&...args)
{
    if (exp)
        return;

    Error(std::move(format), std::forward<Args>(args)...);
}

template <typename... Args>
void Assert(bool exp, std::wformat_string<Args...> format, Args &&...args)
{
    if (exp)
        return;

    Error(std::move(format), std::forward<Args>(args)...);
}
