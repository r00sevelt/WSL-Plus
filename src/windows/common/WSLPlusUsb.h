/*++

Copyright (c) WSL-Plus contributors.

Module Name:

    WSLPlusUsb.h

Abstract:

    WSL-Plus: USB 后端抽象（D1 / 融合矩阵落点）。
    多后端设计（QEMU 哲学）:
      Usbipd    = USB/IP 标准协议（微软官方 WSL 路由，v1 默认）
      Usbredir  = SPICE usbredir 协议（v2 保留，QEMU 生态标准）
      WinUsb    = 用户态直通（v3 条件性，quirks 硬缺口时）
    后端职责: 枚举 / attach(挂载到 VM) / detach(归还)。
    与 D 模型（devices.yaml + 策略）无关 —— 策略层决定"何时调用后端"。

--*/

#pragma once

namespace wsl::windows::common::wslplus::usb
{
    enum class BackendKind
    {
        Usbipd,
    };

    // 后端接口（未来 switch/plugin 式扩展 usbredir/winusb）
    class IUsbBackend
    {
    public:
        virtual ~IUsbBackend() = default;

        virtual BackendKind Kind() const noexcept = 0;

        // 枚举宿主 USB 设备（用于设备目录/规则匹配）
        virtual std::vector<std::string> Enumerate() const = 0;

        // attach: 把指定设备挂到 VM（独占捕获，usbipd attach --busid <id> --device <vmGuid>）
        virtual void Attach(_In_ const std::string& busId, _In_ const GUID& vmId) = 0;

        // detach: 归还（usbipd detach --busid <id>）——弹出语义，硬件回宿主
        virtual void Detach(_In_ const std::string& busId) = 0;

        // 后端是否可用（usbipd 存在）
        virtual bool Available() const noexcept = 0;
    };

    // 构造默认后端（v1 = usbipd-win 进程封装）
    std::unique_ptr<IUsbBackend> CreateDefault();
}
