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
}
