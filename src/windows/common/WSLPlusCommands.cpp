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
        const std::string narrowAction = wsl::shared::string::ToNarrow(action);
        const std::wstring name = (args.size() > 1) ? args[1] : L"";
        const std::string narrowName = wsl::shared::string::ToNarrow(name);

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
    // 后续子命令（profile / ls / file / ...）在此扩展 —— 替换式命令集演进点
    //

    //
    // 未命中 WSL-Plus 子命令 → 交回 WslMain 微软旧逻辑
    //
    return std::nullopt;
}
} // namespace wsl::windows::common::wslplus
