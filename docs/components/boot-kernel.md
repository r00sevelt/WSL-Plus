# 引导与内核机制（boot-kernel）

## 1. WSL 启动链（我们依赖的解剖）
```
Hyper-V/VMP 启动 utility VM
 → 内核直启（无固件——特性; cmdline: initrd=\\LXSS_VM_MODE_INITRD_NAME WSL_ROOT_INIT=1）
 → initrd 内 mini-init（Linux 侧 init 二进制——我们源码 src/linux/init/*）
    核心消息循环: MiniInit 通道（MountDevice/LaunchInit/Snapshot/Resize...）
    格式化(FormatDevice: mkfs.btrfs/@子卷/set-default——S2/A10 改造)
 → 挂载默认子卷(discard,errors=remount-ro; A10 去 subvol=@)
 → 网络/ACL/QoS/隧道/vlan/静态配置（我们的 ApplyXxx 序列）
 → 用户 distro init（ProcessLaunchInitMessage → chroot 起用户系统）
```

## 2. 内核替换指南（.wslconfig）
```
[wsl2]
kernel=\\path\\to\\custom-kernel  # 任意自编/官方 WSL2-Linux-Kernel 构建
```
- 官方内核仓库: microsoft/WSL2-Linux-Kernel（config-wsl: CONFIG_BTRFS_FS=m —— 已验证免编译）
- 我们暂不替换内核（btrfs=模块足）；若未来自研模块→按此路径

## 3. initramfs（我们产 init/initrd.img）
- 生成: tools/create-initrd.ps1（cpio newc; 单文件 init + **E5 扩展**: ExtraToolsDir → 相对路径注入——静态 btrfs-progs 落点）
- msipackage LINUX_BINARIES=init;initrd.img（随 wsl.msi 分发）

## 4. 安全启动/模块签名说明
- VMP 未见强制内核模块签名（微软默认）; 若自编内核照官方流程即可

## 5. 相关改造点清单（启动链内）
FormatDevice(btrfs) | MountOptions(默认子卷) | SetDefaultSubvolume | ApplyExtraNetworkAdapters |
StartExtraDhcpClients | ApplyAclRules/QosRules/StaticAddresses/Tunnels | 唯一化(clone flag) |
Snapshot 消息处理 == 全部集中在 src/linux/init/main.cpp + wslplus_snapshot 模块
