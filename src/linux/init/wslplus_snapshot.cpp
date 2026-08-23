/*++

Copyright (c) WSL-Plus contributors.

Module Name:

    wslplus_snapshot.cpp

Abstract:

    WSL-Plus: btrfs 子卷快照模块实现。根文件系统 = @ 子卷（WSL-Plus S2 改造），
    快照目录 = '/@snap-<name>'（@ 内部的兄弟子卷）。

--*/

#include "wslplus_snapshot.h"
#include <format>
#include <unistd.h>

namespace wslplus_snapshot
{
namespace
{
    // 常量集中（未来配置化点：可改为环境变量/配置文件读取）
    constexpr const char* kBtrfsPath = "/usr/bin/btrfs";
    constexpr const char* kSnapshotPrefix = "/@snap-";       // 相对根（挂载的 @ 子卷）内的快照名
    constexpr const char* kRestoreMarkerDir = "/run/wslplus-rollback-"; // restore 标记（重启生效）
} // namespace

Action ParseAction(const std::string& value)
{
    if (value == "create")
    {
        return Action::Create;
    }
    if (value == "list")
    {
        return Action::List;
    }
    if (value == "delete")
    {
        return Action::Delete;
    }
    if (value == "restore")
    {
        return Action::Restore;
    }
    return Action::Unknown;
}

std::string BuildCommand(Action action, const std::string& name)
{
    switch (action)
    {
    case Action::Create:
        return std::format("{} subvolume snapshot / '{}{}'", kBtrfsPath, kSnapshotPrefix, name);
    case Action::List:
        return std::format("{} subvolume list -p /", kBtrfsPath);
    case Action::Delete:
        return std::format("{} subvolume delete '{}{}'", kBtrfsPath, kSnapshotPrefix, name);
    case Action::Restore:
        // v0.1: 回滚 = 落盘标记（init 启动阶段消费并做子卷交换），避免破坏性在线恢复
        return std::format("touch '{}'", RestoreMarkerPath(name));
    default:
        return "";
    }
}

std::string DefaultSnapshotName()
{
    return std::format("auto-{}", static_cast<long>(getpid()));
}

std::string RestoreMarkerPath(const std::string& name)
{
    return std::format("{}{}", kRestoreMarkerDir, name);
}
} // namespace wslplus_snapshot
