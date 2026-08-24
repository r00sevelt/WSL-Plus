/*++

Copyright (c) WSL-Plus contributors.

Module Name:

    WSLPlusDevices.cpp

Abstract:

    WSL-Plus: 设备管理模型实现（devices.yaml，yaml-cpp）。

--*/

#include "precomp.h"
#include "WSLPlusDevices.h"
#include <yaml-cpp/yaml.h>

namespace wsl::windows::common::wslplus::devices
{
namespace
{
    constexpr const wchar_t* kConfigDir = L".wslplus";
    constexpr const wchar_t* kConfigFile = L"devices.yaml";

    std::string PolicyToString(Policy p)
    {
        switch (p)
        {
        case Policy::Auto:
            return "auto";
        case Policy::Dedicated:
            return "dedicated";
        default:
            return "manual";
        }
    }

    Policy PolicyFromString(const std::string& s)
    {
        if (s == "auto")
        {
            return Policy::Auto;
        }
        if (s == "dedicated")
        {
            return Policy::Dedicated;
        }
        return Policy::Manual;
    }
} // namespace

std::filesystem::path ConfigPath()
{
    return wsl::windows::common::filesystem::GetKnownFolderPath(FOLDERID_UserProfile) / kConfigDir / kConfigFile;
}

std::vector<DeviceConfig> Load()
{
    std::vector<DeviceConfig> result;
    const auto path = ConfigPath();
    if (!std::filesystem::exists(path))
    {
        return result;
    }

    const auto root = YAML::LoadFile(path.string());
    if (!root["devices"])
    {
        return result;
    }

    for (const auto& node : root["devices"])
    {
        DeviceConfig cfg;
        cfg.id = node["id"].as<std::string>("");
        cfg.type = node["type"].as<std::string>("usb");
        cfg.hostPath = node["host"].as<std::string>("");
        cfg.instance = node["instance"].as<std::string>("");
        cfg.policy = PolicyFromString(node["policy"].as<std::string>("manual"));
        result.emplace_back(std::move(cfg));
    }
    return result;
}

void Validate(const DeviceConfig& config)
{
    if (config.id.empty() || config.hostPath.empty())
    {
        THROW_HR(E_INVALIDARG);
    }
    if (config.type != "usb" && config.type != "serial" && config.type != "parallel")
    {
        THROW_HR(E_INVALIDARG);
    }
}

void Save(const DeviceConfig& config)
{
    Validate(config);

    auto path = ConfigPath();
    std::filesystem::create_directories(path.parent_path());

    auto all = Load();
    std::erase_if(all, [&](const DeviceConfig& d) { return d.id == config.id; });
    all.emplace_back(config);

    YAML::Node root;
    for (const auto& d : all)
    {
        YAML::Node node;
        node["id"] = d.id;
        node["type"] = d.type;
        node["host"] = d.hostPath;
        node["instance"] = d.instance;
        node["policy"] = PolicyToString(d.policy);
        root["devices"].push_back(node);
    }

    std::ofstream out(path, std::ios::trunc);
    THROW_LAST_ERROR_IF(!out);
    out << root;
}

void Remove(const std::string& id)
{
    auto all = Load();
    auto kept = all;
    std::erase_if(kept, [&](const DeviceConfig& d) { return d.id == id; });
    if (kept.size() == all.size())
    {
        THROW_HR(E_INVALIDARG);
    }

    auto path = ConfigPath();
    if (kept.empty())
    {
        std::filesystem::remove(path);
        return;
    }

    YAML::Node root;
    for (const auto& d : kept)
    {
        YAML::Node node;
        node["id"] = d.id;
        node["type"] = d.type;
        node["host"] = d.hostPath;
        root["devices"].push_back(node);
    }
    std::ofstream out(path, std::ios::trunc);
    THROW_LAST_ERROR_IF(!out);
    out << root;
}

void SetPolicy(const std::string& id, Policy policy, const std::string& instance)
{
    auto all = Load();
    auto it = std::find_if(all.begin(), all.end(), [&](const DeviceConfig& d) { return d.id == id; });
    THROW_HR_IF(E_INVALIDARG, it == all.end());
    it->policy = policy;
    it->instance = instance;
    Save(*it);
}
} // namespace wsl::windows::common::wslplus::devices
