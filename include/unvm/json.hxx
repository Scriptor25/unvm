#pragma once

#include <unvm/config.hxx>
#include <unvm/version.hxx>

#include <json/json.hxx>

#include <filesystem>

template<>
struct data::serializer<json::node, std::filesystem::path>
{
    static bool from_data(const json::node &node, std::filesystem::path &value);
    static void to_data(json::node &node, const std::filesystem::path &value);
};

template<>
struct data::serializer<json::node, unvm::Config>
{
    static bool from_data(const json::node &node, unvm::Config &value);
    static void to_data(json::node &node, const unvm::Config &value);
};

template<>
struct data::serializer<json::node, unvm::VersionEntry>
{
    static bool from_data(const json::node &node, unvm::VersionEntry &value);
};
