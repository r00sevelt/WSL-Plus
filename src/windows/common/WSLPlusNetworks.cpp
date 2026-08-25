/*++

Copyright (c) WSL-Plus contributors.

Module Name:

    WSLPlusNetworks.cpp

Abstract:

    WSL-Plus: 网络配置数据模型实现（YAML: yaml-cpp 公共依赖，common 已可直接 include）。

--*/

#include "precomp.h"
#include "WSLPlusNetworks.h"
#include <yaml-cpp/yaml.h>

namespace wsl::windows::common::wslplus::networks
{
namespace
{
    constexpr const wchar_t* kConfigDir = L".wslplus";
    constexpr const wchar_t* kConfigFile = L"networks.yaml";
}

std::filesystem::path ConfigPath()
{
    // 用户主目录（userenv API —— precomp 已含头/库, 无 FOLDERID 头链依赖）
    WCHAR profile[MAX_PATH]{};
    DWORD len = MAX_PATH;
    if (!::GetUserProfileDirectoryW(nullptr, profile, &len))
    {
        THROW_LAST_ERROR();
    }
    return std::filesystem::path(profile) / kConfigDir / kConfigFile;
}

static std::vector<NetworkConfig> ParseDocument(const YAML::Node& root)
{
    std::vector<NetworkConfig> result;
    if (!root["networks"])
    {
        return result;
    }

    for (const auto& node : root["networks"])
    {
        NetworkConfig cfg;
        cfg.name = node["name"].as<std::string>("");
        cfg.type = node["type"].as<std::string>("nat");
        cfg.cidr = node["cidr"].as<std::string>("");
        cfg.dns = node["dns"].as<std::string>("");
        if (node["vlan"])
        {
            cfg.vlan = node["vlan"].as<int>();
        }

        if (node["ports"])
        {
            for (const auto& port : node["ports"])
            {
                NetworkPortMapping p;
                p.hostIp = port["host-ip"].as<std::string>("");
                p.hostPort = port["host-port"].as<uint32_t>(0);
                p.guestPort = port["guest-port"].as<uint32_t>(0);
                cfg.ports.emplace_back(p);
            }
        }

        result.emplace_back(std::move(cfg));
    }
    return result;
}

std::vector<NetworkConfig> Load()
{
    const auto path = ConfigPath();
    if (!std::filesystem::exists(path))
    {
        return {};
    }

    return ParseDocument(YAML::LoadFile(path.string()));
}

void Validate(const NetworkConfig& config)
{
    if (config.name.empty())
    {
        THROW_HR(E_INVALIDARG);
    }

    if (config.type != "nat" && config.type != "bridge" && config.type != "host-only")
    {
        THROW_HR(E_INVALIDARG);
    }

    for (const auto& p : config.ports)
    {
        // 端口范围 1-65535（guest 端口必须）
        if (p.guestPort == 0 || p.guestPort > 65535 || p.hostPort > 65535)
        {
            THROW_HR(E_INVALIDARG);
        }
    }
}

void Save(const NetworkConfig& config)
{
    Validate(config);

    auto path = ConfigPath();
    std::filesystem::create_directories(path.parent_path());

    // 读现有 + 同 name 覆盖
    std::vector<NetworkConfig> all = Load();
    std::erase_if(all, [&](const NetworkConfig& c) { return c.name == config.name; });
    all.emplace_back(config);

    YAML::Node root;
    for (const auto& cfg : all)
    {
        YAML::Node node;
        node["name"] = cfg.name;
        node["type"] = cfg.type;
        if (!cfg.cidr.empty())
        {
            node["cidr"] = cfg.cidr;
        }
        if (!cfg.dns.empty())
        {
            node["dns"] = cfg.dns;
        }
        if (cfg.vlan.has_value())
        {
            node["vlan"] = *cfg.vlan;
        }
        if (!cfg.ports.empty())
        {
            for (const auto& p : cfg.ports)
            {
                YAML::Node port;
                if (!p.hostIp.empty())
                {
                    port["host-ip"] = p.hostIp;
                }
                port["host-port"] = p.hostPort;
                port["guest-port"] = p.guestPort;
                node["ports"].push_back(port);
            }
        }
        root["networks"].push_back(node);
    }

    std::ofstream out(path, std::ios::trunc);
    THROW_LAST_ERROR_IF(!out);
    out << root;
}

void Remove(const std::string& name)
{
    auto path = ConfigPath();
    const auto all = Load();
    auto kept = all;
    std::erase_if(kept, [&](const NetworkConfig& c) { return c.name == name; });
    if (kept.size() == all.size())
    {
        THROW_HR(E_INVALIDARG); // 未找到
    }

    if (kept.empty())
    {
        std::filesystem::remove(path);
        return;
    }

    YAML::Node root;
    for (const auto& cfg : kept)
    {
        YAML::Node node;
        node["name"] = cfg.name;
        node["type"] = cfg.type;
        root["networks"].push_back(node);
    }
    std::ofstream out(path, std::ios::trunc);
    THROW_LAST_ERROR_IF(!out);
    out << root;
}

// ---------- C4a: 实例-网络绑定 ----------
namespace
{
    // attachments 区域: { "<instance>": [net1, net2, ...] }
    YAML::Node ReadAttachmentsNode()
    {
        YAML::Node root{};
        const auto path = ConfigPath();
        if (std::filesystem::exists(path))
        {
            root = YAML::LoadFile(path.string());
        }
        return root["attachments"] ? root["attachments"] : YAML::Node(YAML::NodeType::Map);
    }

    void SaveAttachmentsNode(const YAML::Node& attachments)
    {
        auto root = YAML::Node(YAML::NodeType::Map);
        const auto path = ConfigPath();
        if (std::filesystem::exists(path))
        {
            root = YAML::LoadFile(path.string());
        }
        root["attachments"] = attachments;
        std::filesystem::create_directories(path.parent_path());
        std::ofstream out(path, std::ios::trunc);
        THROW_LAST_ERROR_IF(!out);
        out << root;
    }
} // namespace

std::vector<std::string> Attachments(const std::string& instance)
{
    std::vector<std::string> result;
    const auto node = ReadAttachmentsNode();
    if (const auto list = node[instance])
    {
        for (const auto& item : list)
        {
            result.emplace_back(item.as<std::string>());
        }
    }
    return result;
}

void Attach(const std::string& instance, const std::string& network)
{
    auto attachments = ReadAttachmentsNode();
    auto list = attachments[instance];
    if (!list)
    {
        list = YAML::Node(YAML::NodeType::Sequence);
    }
    const std::string dupCheck = network;
    for (const auto& item : list)
    {
        if (item.as<std::string>() == dupCheck)
        {
            return; // 幂等
        }
    }
    list.push_back(network);
    attachments[instance] = list;
    SaveAttachmentsNode(attachments);
}

void Detach(const std::string& instance, const std::string& network)
{
    auto attachments = ReadAttachmentsNode();
    auto list = attachments[instance];
    if (!list)
    {
        return;
    }
    auto kept = YAML::Node(YAML::NodeType::Sequence);
    for (const auto& item : list)
    {
        if (item.as<std::string>() != network)
        {
            kept.push_back(item);
        }
    }
    attachments[instance] = kept;
    SaveAttachmentsNode(attachments);
}
} // namespace wsl::windows::common::wslplus::networks
