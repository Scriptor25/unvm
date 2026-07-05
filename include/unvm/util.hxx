#pragma once

#include <toolkit/result.hxx>
#include <toolkit/string.hxx>

#include <charconv>
#include <filesystem>
#include <format>
#include <iostream>
#include <map>
#include <optional>
#include <ostream>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace unvm
{
    struct Config;

    enum class VersionType
    {
        Default,
        Package,
        Exact,
    };

    std::filesystem::path GetDataDirectory();

    std::istream &GetLine(std::istream &stream, std::string &string, std::string_view delim);

    /**
     * Detect the active version for the current context. Detect versions from package.json, .unvm and global configs.
     *
     * @param def
     * @param type
     * @return
     */
    [[nodiscard]] toolkit::result<std::optional<std::string>> FindActiveVersion(
        const std::optional<std::string> &def,
        VersionType *type = {});

    bool FindVersionFile(std::filesystem::path &path);

    [[nodiscard]] toolkit::result<> ReadVersionFile(std::optional<std::string> &version);
    [[nodiscard]] toolkit::result<> WriteVersionFile(const std::string &version);
    [[nodiscard]] toolkit::result<> RemoveVersionFile();

    template<std::integral T>
    [[nodiscard]] toolkit::result<T> ParseString(
        const std::string &str,
        int base = 10)
    {
        T value;
        auto [_, ec] = std::from_chars(str.data(), str.data() + str.size(), value, base);
        if (ec != std::errc())
        {
            return toolkit::make_error("failed to parse '{}': {}", str, ec);
        }
        return value;
    }

    template<std::floating_point T>
    [[nodiscard]] toolkit::result<T> ParseString(
        const std::string &str,
        std::chars_format fmt = std::chars_format::general)
    {
        T value;
        auto [_, ec] = std::from_chars(str.data(), str.data() + str.size(), value, fmt);
        if (ec != std::errc())
        {
            return toolkit::make_error("failed to parse '{}': {}", str, ec);
        }
        return value;
    }

    std::string GetSSLErrorStack();

    template<typename... M>
    std::string Prompt(const M &... message)
    {
        (std::cout << ... << message) << std::flush;

        std::string input;
        std::getline(std::cin, input);

        return input;
    }

    template<typename... M>
    bool Confirm(const M &... message)
    {
        auto input = toolkit::lowercase(Prompt(message..., " [y/N]: "));

        return input == "y" || input == "yes";
    }

    template<typename C>
    auto PrintString(C &&ctx, std::string_view str)
    {
        for (auto c : str)
        {
            *ctx.out()++ = c;
        }

        return ctx.out();
    }
}

template<typename K, typename V>
std::ostream &operator<<(std::ostream &stream, const std::map<K, V> &map)
{
    stream << "{ ";
    for (auto it = map.begin(); it != map.end(); ++it)
    {
        if (it != map.begin())
        {
            stream << ", ";
        }
        stream << it->first << ": " << it->second;
    }
    return stream << " }";
}

template<typename T>
struct std::formatter<std::set<T>>
{
    template<typename C>
    constexpr auto parse(C &&ctx)
    {
        return std::forward<C>(ctx).begin();
    }

    template<typename C>
    auto format(const std::set<T> &value, C &&ctx) const
    {
        std::vector<std::string> segments;
        for (auto it = value.begin(); it != value.end(); ++it)
        {
            segments.push_back(std::format("{}", *it));
        }

        std::string str;
        for (auto it = segments.begin(); it != segments.end(); ++it)
        {
            if (it != segments.begin())
            {
                if (it != segments.end() - 1)
                {
                    str += ", ";
                }
                else
                {
                    str += " or ";
                }
            }

            str += *it;
        }

        return unvm::PrintString(ctx, str);
    }
};
