/*++

Copyright (c) WSL-Plus contributors.

Module Name:

    WSLPlusNetworks.h

Abstract:

    WSL-Plus: 网络配置数据模型（C1）—— networks.yaml 的读写与校验。
    存储: %USERPROFILE%\.wslplus\networks.yaml
    结构: networks: [{name, type(nat|bridge|host-only), cidr, dns, ports:[{host,guest}]}]
    设计: 模块只做"配置域"（文件 CRUD+校验）；HCN 编排在 C3/C4 接入（数据流单向）。

--*/

#pragma once

#include <optional>
#include <string>
#include <vector>

namespace wsl::windows::common::wslplus::networks
{
    struct NetworkPortMapping
    {
        std::string hostIp;
        uint32_t hostPort = 0;
        uint32_t guestPort = 0;
        bool operator==(const NetworkPortMapping&) const = default;
    };

    struct NetworkConfig
    {
        std::string name;
        std::string type = "nat"; // nat | bridge | host-only
        std::string cidr;         // 如 192.168.100.0/24
        std::string dns;          // 如 192.168.100.1
        std::optional<int> vlan;  // C6: VLAN id（guest 侧 vlan 子接口，服务端透明）
        std::vector<NetworkPortMapping> ports;
        bool operator==(const NetworkConfig&) const = default;
    };

    // 配置路径（用户级，自动建目录）
    std::filesystem::path ConfigPath();

    // 读取全部网络（文件不存在 → 空列表；解析/校验失败 → throw）
    std::vector<NetworkConfig> Load();

    // 追加/更新一个网络（同 name 覆盖，端口去重）；保存到文件
    void Save(const NetworkConfig& config);

    // 按 name 删除；不存在 → throw
    void Remove(const std::string& name);

    // 校验器（名称唯一/src 合法/端口范围），供 CLI 与未来 HCN 层复用
    void Validate(const NetworkConfig& config);

    // C4a: 实例-网络绑定关系（instance -> [network names]）
    std::vector<std::string> Attachments(const std::string& instance);
    void Attach(const std::string& instance, const std::string& network);
    void Detach(const std::string& instance, const std::string& network);
}
