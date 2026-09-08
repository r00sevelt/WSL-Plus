/*++

Copyright (c) WSL-Plus contributors.

Module Name:

    WSLPlusCommands.h

Abstract:

    WSL-Plus Incus 风格子命令分发入口（替换式命令集骨架）。
    未命中任何 WSL-Plus 子命令时返回 nullopt，由 WslMain 继续微软旧逻辑。

--*/

#pragma once

namespace wsl::windows::common::wslplus
{
    // 返回该命令的退出码；若 argv 不匹配任何 WSL-Plus 子命令则返回 nullopt。
    std::optional<int> Dispatch(_In_ const std::wstring& commandLine);

    // WSL-Plus (ADR-14): 高危命令统一确认门。
    // TTY 交互 = type-to-confirm（输入资源名确认）；--yes = 跳过；非交互（脚本/管道）= 警告不阻断。
    // what: 危险动作描述（如 L"unregister" / L"snapshot delete"）；name: 受影响资源名。
    bool ConfirmDestructive(_In_ const std::wstring& what, _In_ const std::wstring& name, bool assumeYes);
}
