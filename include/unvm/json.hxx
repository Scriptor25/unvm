#pragma once

#include <unvm/config.hxx>
#include <unvm/version.hxx>

#include <json/json.hxx>

template<>
bool from_json(const json::Node &node, std::filesystem::path &value);

template<>
void to_json(json::Node &node, const std::filesystem::path &value);

template<>
bool from_json(const json::Node &node, unvm::Config &value);

template<>
void to_json(json::Node &node, const unvm::Config &value);

template<>
bool from_json(const json::Node &node, unvm::StringOrFalse &value);

template<>
bool from_json(const json::Node &node, unvm::VersionEntry &value);
