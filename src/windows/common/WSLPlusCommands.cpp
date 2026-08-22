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
    void PrintStub(_In_ LPCWSTR commandLine, const std::vector<std::wstring>& args)
    {
        std::wstring message = L"WSL-Plus: '" + std::wstring(commandLine) + L"' 骨架就绪（服务端路由 S4 接入）：";
        for (const auto& arg : args)
        {
            message += L" ";
            message += L"'" + arg + L"'";
        }
        wsl::windows::common::wslutil::PrintMessage(message);
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
            PrintStub(L"snapshot create", args);
            return 0;
        }
        else if (action == L"list")
        {
            PrintStub(L"snapshot list", args);
            return 0;
        }
        else if (action == L"restore")
        {
            PrintStub(L"snapshot restore", args);
            return 0;
        }
        else if (action == L"delete")
        {
            PrintStub(L"snapshot delete", args);
            return 0;
        }

        PrintSnapshotUsage();
        return -1;
    }

    //
    // 后续子命令（clone / profile / ls / file / ...）在此扩展 —— 替换式命令集演进点
    //

    //
    // 未命中 WSL-Plus 子命令 → 交回 WslMain 微软旧逻辑
    //
    return std::nullopt;
}
} // namespace wsl::windows::common::wslplus
