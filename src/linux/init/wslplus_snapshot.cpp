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
        // v0.2: 回滚 = 默认子卷切换（set-default 指向快照 → 重启后活动根=快照）
        // 此前自动保底: 先把当前 @ 快照为 pre-restore-<name>（只进不退）
        return std::format(
            "{0} subvolume snapshot / '/@snap-pre-restore-{1}'; {2}",
            kBtrfsPath, name, BuildSetDefaultCommand(std::format("/@snap-{}", name)));
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

std::string BuildSetDefaultCommand(const std::string& subvolPath)
{
    // 取子卷 ID → set-default（挂载不指定 subvol 时生效；重启后活动根=该子卷）
    return std::format(
        "ID=$({} subvolume show '{}' | awk '/Subvolume ID/{{print $3}}'); {} subvolume set-default $ID /",
        kBtrfsPath, subvolPath, kBtrfsPath);
}
} // namespace wslplus_snapshot
