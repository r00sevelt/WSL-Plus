# 组件: wslplus_snapshot（guest 快照模块）

## 概述
Linux 侧 btrfs 快照逻辑（独立模块——Windows 只发 action+name，btrfs 细节集中于此；可独立测试）。根文件系统=@ 子卷（S2 改造），快照目录='/@snap-<name>'。

## 位置
`src/linux/init/wslplus_snapshot.{h,cpp}`（init 二进制的源码集）

## 公共 API
```cpp
enum class Action { Unknown, Create, List, Delete, Restore };
Action ParseAction(const std::string&);            // 协议值→枚举
std::string BuildCommand(Action, const std::string& name);
   // Create:  /usr/bin/btrfs subvolume snapshot / '/@snap-<name>'
   // List:    btrfs subvolume list -p /
   // Delete:  btrfs subvolume delete '/@snap-<name>'
   // Restore: 保底快照 pre-restore-<name> + set-default 链（重启切根）
std::string DefaultSnapshotName();                 // auto-<pid>
std::string RestoreMarkerPath(name);               // /run/wslplus-rollback-<name>
std::string BuildSetDefaultCommand(subvolPath);    // ID=$(show|awk); set-default $ID /
```
常量表集中: kBtrfsPath=/usr/bin/btrfs · kSnapshotPrefix=/@snap-

## 数据流（端到端）
MiniInit 消息(Snapshot) → main.cpp ProcessSnapshotMessage（fork 子进程执行 BuildCommand 命令 + 响应码回传; list 输出经 OutputSocket 管道）

## 扩展点
- v0.2 恢复真正"原地替换"（set-default 已就位; 挂载默认子卷=已完成）
- 工具注入: mkfs.btrfs 静态化走 E5(create-initrd 接口已备)

## 测试归属
T1（快照全链, 纯命令可在任意 btrfs 容器直接验）
