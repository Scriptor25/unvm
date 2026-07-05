#pragma once

#include <unvm/config.hxx>
#include <unvm/version.hxx>

#include <json/json.hxx>

#include <filesystem>

template<>
struct data::serializer<std::filesystem::path>
{
    static bool from_data(const json::node_t &node, std::filesystem::path &value);
    static void to_data(json::node_t &node, const std::filesystem::path &value);
};

template<>
struct data::serializer<unvm::Config>
{
    static bool from_data(const json::node_t &node, unvm::Config &value);
    static void to_data(json::node_t &node, const unvm::Config &value);
};

template<>
struct data::serializer<unvm::VersionEntry>
{
    static bool from_data(const json::node_t &node, unvm::VersionEntry &value);
};
