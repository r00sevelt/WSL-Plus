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

namespace wsl::windows::common::wslplus
{
namespace
{
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
            auto it = std::find_if(networks.begin(), networks.end(), [&](const auto& n) { return n.name == argv[3]; });
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
            cfg.name = argv[3];
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
                wsl::windows::common::wslplus::networks::Remove(argv[3]);
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

            wsl::windows::common::wslplus::networks::Attach(argv[3], argv[4]);
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

            wsl::windows::common::wslplus::networks::Detach(argv[3], argv[4]);
            wsl::windows::common::wslutil::PrintMessage(L"WSL-Plus: 网络已解绑");
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
