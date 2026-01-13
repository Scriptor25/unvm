#pragma once

#include <format>
#include <iostream>

template <typename... Args>
[[noreturn]] void Error(std::format_string<Args...> format, Args &&...args)
{
    auto message = std::format(format, args...);
    std::cerr << message << std::endl;

    exit(1);
}

template <typename... Args>
[[noreturn]] void Error(std::wformat_string<Args...> format, Args &&...args)
{
    auto message = std::format(format, args...);
    std::wcerr << message << std::endl;

    exit(1);
}

template <typename... Args>
void Assert(bool exp, std::format_string<Args...> format, Args &&...args)
{
    if (exp)
        return;

    Error(format, args...);
}
