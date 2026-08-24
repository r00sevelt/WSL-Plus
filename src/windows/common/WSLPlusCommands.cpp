/*++

Copyright (c) WSL-Plus contributors.

Module Name:

    WSLPlusCommands.cpp

Abstract:

    WSL-Plus Incus 风格子命令分发实现（骨架）。
    L3: snapshot create/list/restore/delete 已注册解析；服务端路由（S4 接入）。

--*/

#include "precomp.h"
#include "WSLPlusCommands.h"
#include "WSLPlusNetworks.h"
#include "WSLPlusImages.h"
#include "WSLPlusDevices.h"
#include "WSLPlusUsb.h"

namespace wsl::windows::common::wslplus
{
namespace
{
    // 统一窄化助手（CLI argv 为 LPWSTR；模块接口为 std::string —— 一处定义全文件使用）
    std::string Narrow(_In_ LPCWSTR value)
    {
        return wsl::windows::common::string::WideToMultiByte(value);
    }

    void PrintSnapshotUsage(); // 前向声明（定义在 ExecuteSnapshot 之后）

    // WSL-Plus: 快照命令执行（CLI→SvcComm→服务端→guest btrfs 模块）
    int ExecuteSnapshot(_In_ LPCWSTR action, _In_ const std::vector<std::wstring>& args)
    {
        if (args.empty() || args[0].empty())
        {
            PrintSnapshotUsage();
            return -1;
        }

        wsl::windows::common::SvcComm service;
        const auto distroId = service.GetDistributionId(args[0].c_str());
        const std::string narrowAction = wsl::windows::common::string::WideToMultiByte(action);
        const std::wstring name = (args.size() > 1) ? args[1] : L"";
        const std::string narrowName = wsl::windows::common::string::WideToMultiByte(name);

        const HRESULT result = service.SnapshotDistribution(&distroId, narrowAction.c_str(), narrowName.c_str());
        if (FAILED(result))
        {
            wsl::windows::common::wslutil::PrintMessage(wsl::windows::common::wslutil::GetSystemErrorString(result));
            return -1;
        }

        wsl::windows::common::wslutil::PrintMessage(L"WSL-Plus snapshot 命令已提交");
        return 0;
    }

    void PrintSnapshotUsage()
    {
        wsl::windows::common::wslutil::PrintMessage(
            L"用法: wsl snapshot create <instance> [name]\n"
            L"      wsl snapshot list [instance]\n"
            L"      wsl snapshot restore <instance> <name>\n"
            L"      wsl snapshot delete <instance> [name]");
    }
} // namespace

std::optional<int> Dispatch(_In_ const std::wstring& commandLine)
{
    int argc = 0;
    const wil::unique_hlocal_ptr<LPWSTR[]> argv{CommandLineToArgvW(commandLine.c_str(), &argc)};
    THROW_LAST_ERROR_IF(!argv);

    if (argc < 2)
    {
        return std::nullopt;
    }

    const std::wstring_view verb(argv[1]);

    //
    // snapshot 子命令组（P1-A 快照；服务端路由 S4 接入）
    //
    if (verb == L"snapshot")
    {
        if (argc < 3)
        {
            PrintSnapshotUsage();
            return 0;
        }

        const std::wstring_view action(argv[2]);
        std::vector<std::wstring> args;
        for (int i = 3; i < argc; ++i)
        {
            args.emplace_back(argv[i]);
        }

        if (action == L"create")
        {
            return ExecuteSnapshot(L"create", args);
        }
        else if (action == L"list")
        {
            return ExecuteSnapshot(L"list", args);
        }
        else if (action == L"restore")
        {
            return ExecuteSnapshot(L"restore", args);
        }
        else if (action == L"delete")
        {
            return ExecuteSnapshot(L"delete", args);
        }

        PrintSnapshotUsage();
        return -1;
    }

    //
    // clone 子命令组（P1-B 克隆: 链接克隆=默认 / --full=完整克隆）
    //
    if (verb == L"clone")
    {
        if (argc < 4)
        {
            wsl::windows::common::wslutil::PrintMessage(
                L"用法: wsl clone <src> <dst> [--full]（默认链接克隆=COW 差异盘；--full=独立副本）");
            return -1;
        }

        bool fullClone = false;
        for (int i = 3; i < argc; ++i)
        {
            if (std::wstring_view(argv[i]) == L"--full")
            {
                fullClone = true;
            }
        }

        wsl::windows::common::SvcComm service;
        const auto distroId = service.GetDistributionId(argv[2]);
        const HRESULT result = service.CloneDistribution(&distroId, argv[3], fullClone);
        if (FAILED(result))
        {
            wsl::windows::common::wslutil::PrintMessage(wsl::windows::common::wslutil::GetSystemErrorString(result));
            return -1;
        }

        wsl::windows::common::wslutil::PrintMessage(L"WSL-Plus clone 命令已提交");
        return 0;
    }

    //
    // network 子命令组（C1: 网络配置数据模型; wsl network ls/add/rm）
    //
    if (verb == L"network")
    {
        if (argc < 3)
        {
            wsl::windows::common::wslutil::PrintMessage(
                L"用法: wsl network ls | wsl network add <name> [--type nat|bridge|host-only] [--cidr x] | wsl network rm <name>");
            return 0;
        }

        const std::wstring_view netAction(argv[2]);
        if (netAction == L"show")
        {
            if (argc < 4)
            {
                wsl::windows::common::wslutil::PrintMessage(L"用法: wsl network show <name>");
                return -1;
            }
            const auto networks = wsl::windows::common::wslplus::networks::Load();
            const auto showName = Narrow(argv[3]);
            auto it = std::find_if(networks.begin(), networks.end(), [&](const auto& n) { return n.name == showName; });
            if (it == networks.end())
            {
                wsl::windows::common::wslutil::PrintMessage(L"WSL-Plus: 网络不存在");
                return -1;
            }
            wsl::windows::common::wslutil::PrintMessage(
                std::format(L"{} ({})  {}  dns={}  ports={}", it->name, it->type, it->cidr, it->dns, it->ports.size()));
            for (const auto& p : it->ports)
            {
                wsl::windows::common::wslutil::PrintMessage(
                    std::format(L"  {}:{} -> guest:{}", p.hostIp, p.hostPort, p.guestPort));
            }
            return 0;
        }
        else if (netAction == L"ls")
        {
            const auto networks = wsl::windows::common::wslplus::networks::Load();
            for (const auto& n : networks)
            {
                wsl::windows::common::wslutil::PrintMessage(
                    std::format(L"{} ({})  {} {}  ports={}", n.name, n.type, n.cidr, n.dns, n.ports.size()));
            }
            return 0;
        }
        else if (netAction == L"add")
        {
            if (argc < 4)
            {
                wsl::windows::common::wslutil::PrintMessage(L"用法: wsl network add <name> [--type nat|bridge|host-only] [--cidr x.x.x.x/nn]");
                return -1;
            }

            wsl::windows::common::wslplus::networks::NetworkConfig cfg;
            cfg.name = Narrow(argv[3]);
            for (int i = 4; i < argc; ++i)
            {
                if (std::wstring_view(argv[i]) == L"--type" && i + 1 < argc)
                {
                    cfg.type = wsl::windows::common::string::WideToMultiByte(argv[++i]);
                }
                else if (std::wstring_view(argv[i]) == L"--cidr" && i + 1 < argc)
                {
                    cfg.cidr = wsl::windows::common::string::WideToMultiByte(argv[++i]);
                }
                else if (std::wstring_view(argv[i]) == L"--vlan" && i + 1 < argc)
                {
                    cfg.vlan = _wtoi(argv[++i]);
                }
            }

            try
            {
                wsl::windows::common::wslplus::networks::Save(cfg);
                wsl::windows::common::wslutil::PrintMessage(L"WSL-Plus network 已保存");
                return 0;
            }
            catch (...)
            {
                wsl::windows::common::wslutil::PrintMessage(L"WSL-Plus: 网络配置无效");
                return -1;
            }
        }
        else if (netAction == L"rm")
        {
            if (argc < 4)
            {
                wsl::windows::common::wslutil::PrintMessage(L"用法: wsl network rm <name>");
                return -1;
            }

            try
            {
                wsl::windows::common::wslplus::networks::Remove(Narrow(argv[3]));
                wsl::windows::common::wslutil::PrintMessage(L"WSL-Plus network 已删除");
                return 0;
            }
            catch (...)
            {
                wsl::windows::common::wslutil::PrintMessage(L"WSL-Plus: 网络不存在");
                return -1;
            }
        }
        else if (netAction == L"attach")
        {
            if (argc < 5)
            {
                wsl::windows::common::wslutil::PrintMessage(L"用法: wsl network attach <instance> <network>");
                return -1;
            }

            wsl::windows::common::wslplus::networks::Attach(Narrow(argv[3]), Narrow(argv[4]));
            wsl::windows::common::wslutil::PrintMessage(L"WSL-Plus: 网络已绑定实例");
            return 0;
        }
        else if (netAction == L"detach")
        {
            if (argc < 5)
            {
                wsl::windows::common::wslutil::PrintMessage(L"用法: wsl network detach <instance> <network>");
                return -1;
            }

            wsl::windows::common::wslplus::networks::Detach(Narrow(argv[3]), Narrow(argv[4]));
            wsl::windows::common::wslutil::PrintMessage(L"WSL-Plus: 网络已解绑");
            return 0;
        }
        else if (netAction == L"ports-on")
        {
            // C3b: 应用实例关联网络的 ports（宿主监听 → VM 转发）
            if (argc < 4)
            {
                wsl::windows::common::wslutil::PrintMessage(L"用法: wsl network ports-on <instance>");
                return -1;
            }

            const auto allNetworks = wsl::windows::common::wslplus::networks::Load();
            const auto attached = wsl::windows::common::wslplus::networks::Attachments(Narrow(argv[3]));

            nlohmann::json portsJson = nlohmann::json::array();
            for (const auto& netName : attached)
            {
                auto it = std::find_if(allNetworks.begin(), allNetworks.end(), [&](const auto& n) { return n.name == netName; });
                if (it == allNetworks.end())
                {
                    continue;
                }
                for (const auto& p : it->ports)
                {
                    portsJson.push_back({{"hostPort", p.hostPort}, {"guestPort", p.guestPort}, {"bindingIp", p.hostIp}});
                }
            }

            wsl::windows::common::SvcComm service;
            const auto distroId = service.GetDistributionId(argv[3]);
            const std::string jsonText = portsJson.dump();
            const HRESULT result = service.ApplyPortMappings(&distroId, jsonText.c_str());
            if (FAILED(result))
            {
                wsl::windows::common::wslutil::PrintMessage(wsl::windows::common::wslutil::GetSystemErrorString(result));
                return -1;
            }

            wsl::windows::common::wslutil::PrintMessage(L"WSL-Plus: 端口映射已启用");
            return 0;
        }
    }

    //
    // image 子命令组（D3: 本地镜像库; wsl image list/import/rm）
    //
    if (verb == L"image")
    {
        if (argc < 3)
        {
            wsl::windows::common::wslutil::PrintMessage(
                L"用法: wsl image list | wsl image import <tar> <name> [--desc x] | wsl image rm <name>");
            return 0;
        }

        const std::wstring_view imgAction(argv[2]);
        if (imgAction == L"list")
        {
            const auto images = wsl::windows::common::wslplus::images::List();
            for (const auto& img : images)
            {
                wsl::windows::common::wslutil::PrintMessage(
                    std::format(L"{}  ({} / {})",
                        wsl::windows::common::string::MultiByteToWide(img.name),
                        wsl::windows::common::string::MultiByteToWide(img.architecture),
                        wsl::windows::common::string::MultiByteToWide(img.baseVersion)));
            }
            return 0;
        }
        else if (imgAction == L"import")
        {
            if (argc < 5)
            {
                wsl::windows::common::wslutil::PrintMessage(L"用法: wsl image import <tar路径> <name> [--desc 描述]");
                return -1;
            }

            const std::wstring desc = [&]() -> std::wstring {
                for (int i = 4; i < argc; ++i)
                {
                    if (std::wstring_view(argv[i]) == L"--desc" && i + 1 < argc)
                    {
                        return argv[i + 1];
                    }
                }
                return L"";
            }();

            try
            {
                auto manifest = wsl::windows::common::wslplus::images::Import(argv[3], argv[4], desc.empty() ? nullptr : desc.c_str());
                wsl::windows::common::wslutil::PrintMessage(
                    std::format(L"WSL-Plus 镜像已导入: {}", manifest.name));
                return 0;
            }
            catch (...)
            {
                wsl::windows::common::wslutil::PrintMessage(L"WSL-Plus: 镜像导入失败");
                return -1;
            }
        }
        else if (imgAction == L"rm")
        {
            if (argc < 4)
            {
                wsl::windows::common::wslutil::PrintMessage(L"用法: wsl image rm <name>");
                return -1;
            }

            try
            {
                wsl::windows::common::wslplus::images::Remove(argv[3]);
                wsl::windows::common::wslutil::PrintMessage(L"WSL-Plus 镜像已删除");
                return 0;
            }
            catch (...)
            {
                wsl::windows::common::wslutil::PrintMessage(L"WSL-Plus: 镜像不存在");
                return -1;
            }
        }
    }

    //
    // device 子命令组（D 组: 设备管理模型; wsl device list/add/rm/auto）
    //
    if (verb == L"device")
    {
        if (argc < 3)
        {
            wsl::windows::common::wslutil::PrintMessage(
                L"用法: wsl device list | wsl device add <id> <type usb|serial(COM串口)|parallel(LPT并口)> <host> [--instance x --policy auto|manual|dedicated] | wsl device rm <id>");
            return 0;
        }

        const std::wstring_view devAction(argv[2]);
        if (devAction == L"list")
        {
            const auto devices = wsl::windows::common::wslplus::devices::Load();
            for (const auto& d : devices)
            {
                wsl::windows::common::wslutil::PrintMessage(
                    std::format(L"{} ({}) {} -> {} [{}]", d.id, d.type, d.hostPath, d.instance, d.policy == wsl::windows::common::wslplus::devices::Policy::Auto ? L"auto" : (d.policy == wsl::windows::common::wslplus::devices::Policy::Dedicated ? L"dedicated" : L"manual")));
            }
            return 0;
        }
        else if (devAction == L"add")
        {
            if (argc < 6)
            {
                wsl::windows::common::wslutil::PrintMessage(L"用法: wsl device add <id> <type> <host> [--instance x --policy auto|manual|dedicated]");
                return -1;
            }

            wsl::windows::common::wslplus::devices::DeviceConfig cfg;
            cfg.id = Narrow(argv[3]);
            cfg.type = wsl::windows::common::string::WideToMultiByte(argv[4]);
            cfg.hostPath = wsl::windows::common::string::WideToMultiByte(argv[5]);
            for (int i = 6; i < argc; ++i)
            {
                if (std::wstring_view(argv[i]) == L"--instance" && i + 1 < argc)
                {
                    cfg.instance = wsl::windows::common::string::WideToMultiByte(argv[++i]);
                }
                else if (std::wstring_view(argv[i]) == L"--policy" && i + 1 < argc)
                {
                    const auto p = wsl::windows::common::string::WideToMultiByte(argv[++i]);
                    cfg.policy = (p == "auto") ? wsl::windows::common::wslplus::devices::Policy::Auto
                        : (p == "dedicated")                            ? wsl::windows::common::wslplus::devices::Policy::Dedicated
                                                                        : wsl::windows::common::wslplus::devices::Policy::Manual;
                }
            }

            try
            {
                wsl::windows::common::wslplus::devices::Save(cfg);
                wsl::windows::common::wslutil::PrintMessage(L"WSL-Plus 设备已注册");
                return 0;
            }
            catch (...)
            {
                wsl::windows::common::wslutil::PrintMessage(L"WSL-Plus: 设备配置无效");
                return -1;
            }
        }
        else if (devAction == L"rm")
        {
            // 安全说明: 仅删除登记记录（不触碰硬件/驱动/Windows 侧）
            if (argc < 4)
            {
                wsl::windows::common::wslutil::PrintMessage(L"用法: wsl device rm <id>（仅清理登记，不碰硬件）");
                return -1;
            }
            wsl::windows::common::wslplus::devices::Remove(Narrow(argv[3]));
            wsl::windows::common::wslutil::PrintMessage(L"WSL-Plus 设备登记已删除");
            return 0;
        }
        else if (devAction == L"auto")
        {
            if (argc < 4)
            {
                wsl::windows::common::wslutil::PrintMessage(L"用法: wsl device auto <id> <instance>");
                return -1;
            }
            wsl::windows::common::wslplus::devices::SetPolicy(Narrow(argv[3]), wsl::windows::common::wslplus::devices::Policy::Auto, Narrow(argv[4]));
            wsl::windows::common::wslutil::PrintMessage(L"WSL-Plus 设备已设为自动共享");
            return 0;
        }
        else if (devAction == L"attach")
        {
            // D 组: 挂载设备到实例（serial → 服务端串口重定向通道真挂载）
            if (argc < 5)
            {
                wsl::windows::common::wslutil::PrintMessage(L"用法: wsl device attach <id> <instance>");
                return -1;
            }

            const auto devices = wsl::windows::common::wslplus::devices::Load();
            const auto attachId = Narrow(argv[3]);
            auto it = std::find_if(devices.begin(), devices.end(), [&](const auto& d) { return d.id == attachId; });
            if (it == devices.end())
            {
                wsl::windows::common::wslutil::PrintMessage(L"WSL-Plus: 设备未登记（先 wsl device add）");
                return -1;
            }

            if (it->type == "serial")
            {
                wsl::windows::common::SvcComm service;
                const auto distroId = service.GetDistributionId(argv[4]);
                return SUCCEEDED(service.AttachSerialPort(&distroId, it->hostPath.c_str())) ? 0 : -1;
            }

            if (it->type == "parallel")
            {
                // 并口通道: USB-LPT 桥接设备走 USB 后端（host=usb 设备标识）；
                // 原生 ISA LPT 不支持（现代平台无直通路径）
                if (!it->hostPath.empty())
                {
                    auto backend = wsl::windows::common::wslplus::usb::CreateDefault();
                    wsl::windows::common::SvcComm service;
                    const auto distroId = service.GetDistributionId(argv[4]);
                    backend->Attach(it->hostPath, distroId);
                    return 0;
                }
                wsl::windows::common::wslutil::PrintMessage(L"WSL-Plus: 并口需 USB-LPT 桥接设备（host=USB id）");
                return -1;
            }

            wsl::windows::common::wslutil::PrintMessage(L"WSL-Plus: USB 类型请用 wsl device watch/后端通道");
            return -1;
        }
        else if (devAction == L"watch")
        {
            // D1-2: 热插拔守护（Ctrl+C 退出）—— new USB 设备 → 匹配 auto 策略登记 → 自动 attach
            wsl::windows::common::wslutil::PrintMessage(L"WSL-Plus: USB 热插拔守护启动（Ctrl+C 停止）");
            auto backend = wsl::windows::common::wslplus::usb::CreateDefault();
            wsl::windows::common::wslplus::usb::WatchLoop(
                backend, 5,
                [](const std::string& busId) -> bool
                {
                    const auto devices = wsl::windows::common::wslplus::devices::Load();
                    auto it = std::find_if(devices.begin(), devices.end(), [&](const auto& d)
                                           { return d.type == "usb" && d.hostPath == busId && d.policy == wsl::windows::common::wslplus::devices::Policy::Auto && !d.instance.empty(); });
                    if (it == devices.end())
                    {
                        return true;
                    }

                    wsl::windows::common::SvcComm service;
                    const auto distroId = service.GetDistributionId(it->instance.c_str());
                    // 后端 attach: busId 直传（usbipd 侧通常需要 DEVICE_ID；busId 兼容 usbipd list 输出）
                    wsl::windows::common::wslplus::usb::CreateDefault()->Attach(busId, distroId);
                    return true;
                });
            return 0;
        }
        else if (devAction == L"eject")
        {
            // D 组安全语义: 归还 Windows（弹出式取消挂载）——
            // serial: 服务端断桥归还；同时松绑登记（manual 无实例）
            if (argc < 4)
            {
                wsl::windows::common::wslutil::PrintMessage(L"用法: wsl device eject <id>（归还 Windows，类似弹出 U 盘）");
                return -1;
            }

            const auto devices = wsl::windows::common::wslplus::devices::Load();
            const auto ejectId = Narrow(argv[3]);
            auto it = std::find_if(devices.begin(), devices.end(), [&](const auto& d) { return d.id == ejectId; });
            if (it != devices.end() && it->type == "serial" && !it->instance.empty())
            {
                wsl::windows::common::SvcComm service;
                const auto distroId = service.GetDistributionId(it->instance.c_str());
                service.DetachSerialPort(&distroId);
            }

            wsl::windows::common::wslplus::devices::SetPolicy(Narrow(argv[3]), wsl::windows::common::wslplus::devices::Policy::Manual, L"");
            wsl::windows::common::wslutil::PrintMessage(L"WSL-Plus: 设备已归还 Windows（可再 attach 挂载）");
            return 0;
        }
    }

    //
    // 后续子命令（profile / ls / file / ...）在此扩展 —— 替换式命令集演进点
    //

    //
    // 未命中 WSL-Plus 子命令 → 交回 WslMain 微软旧逻辑
    //
    return std::nullopt;
}
} // namespace wsl::windows::common::wslplus
