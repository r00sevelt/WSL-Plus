/*++

Copyright (c) WSL-Plus contributors.

Module Name:

    WSLPlusDevices.h

Abstract:

    WSL-Plus: 设备管理模型（D 组统一）—— 设备目录与实例绑定策略。
    策略三档（VMware/VBox/Parallels/QEMU 同哲学）:
      auto      = 自动共享（实例启动时控制面自动挂载，仅可共享类）
      manual    = 手动连接（用户显式"连接实例"，默认）
      dedicated = 直通专用（独占硬件，仅显式开启）
    存储: %USERPROFILE%\.wslplus\devices.yaml
    结构: devices: [{id, type(usb|serial|parallel), host, instance, policy}]

--*/

#pragma once

namespace wsl::windows::common::wslplus::devices
{
    enum class Policy
    {
        Manual,    // 默认
        Auto,
        Dedicated,
    };

    struct DeviceConfig
    {
        std::string id;       // 宿主设备标识（如 COM3 / usb:VID:PID）
        std::string type;     // usb | serial | parallel
        std::string hostPath; // 宿主路径/端口名（串口=COMxx USB=标识）
        std::string instance; // 绑定的 WSL 实例（manual/dedicated）
        Policy policy = Policy::Manual;
        bool operator==(const DeviceConfig&) const = default;
    };

    std::filesystem::path ConfigPath();

    // 全量设备目录（文件不存在 → 空）
    std::vector<DeviceConfig> Load();

    void Save(const DeviceConfig& config);      // 同 id 覆盖
    void Remove(const std::string& id);          // 不存在 → throw
    void SetPolicy(const std::string& id, Policy policy, const std::string& instance);
    void Validate(const DeviceConfig& config);
}
