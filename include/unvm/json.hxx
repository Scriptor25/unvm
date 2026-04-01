#pragma once

#include <unvm/config.hxx>
#include <unvm/version.hxx>

#include <json/json.hxx>

template<>
struct json::serializer<std::filesystem::path>
{
    static bool from_json(const Node &node, std::filesystem::path &value);
    static void to_json(Node &node, const std::filesystem::path &value);
};

template<>
struct json::serializer<unvm::Config>
{
    static bool from_json(const Node &node, unvm::Config &value);
    static void to_json(Node &node, const unvm::Config &value);
};

template<>
struct json::serializer<unvm::StringOrFalse>
{
    static bool from_json(const Node &node, unvm::StringOrFalse &value);
};

template<>
struct json::serializer<unvm::VersionEntry>
{
    static bool from_json(const Node &node, unvm::VersionEntry &value);
};
