/*++

Copyright (c) WSL-Plus contributors.

Module Name:

    wslplus_snapshot.h

Abstract:

    WSL-Plus: btrfs 子卷快照模块（guest 侧）。快照/枚举/删除/回滚标记的命令构造集中于此，
    逻辑与 Windows 侧完全解耦（协议只传 action + name），便于独立测试与后端可替换。

--*/

#pragma once

#include <string>

namespace wslplus_snapshot
{
    // 快照动作（与 Windows 侧消息动作字符串一一对应）
    enum class Action
    {
        Unknown,
        Create,
        List,
        Delete,
        Restore,
    };

    // 由字符串解析动作（协议值）
    Action ParseAction(const std::string& value);

    // 构造待执行命令（btrfs 细节隔离在模块内；路径常量集中于 k* 表，便于未来配置化）
    std::string BuildCommand(Action action, const std::string& name);

    // 快照命名规则（集中；默认 auto-<pid>）
    std::string DefaultSnapshotName();

    // 回滚标记路径（restore 落盘；v0.2 起恢复走"默认子卷切换"，此函数保留为标记型备选）
    std::string RestoreMarkerPath(const std::string& name);

    // v0.2: 默认子卷切换命令文本——
    //   安装时: BuildSetDefaultCommand("/@") 使挂载默认=@；
    //   restore: BuildSetDefaultCommand("/@snap-x") 重启后活动根=快照
    // 实现: subvolume show 取 Subvolume ID → btrfs subvolume set-default <id> <dev>
    std::string BuildSetDefaultCommand(const std::string& subvolPath);
}
