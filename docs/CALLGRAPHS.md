# 关键功能调用链（从"一条命令"到"代码"——查这个比 grep 快）

## 快照 create/list/restore/delete
```
wsl.exe(snapshot)                          → src/windows/common/WSLPlusCommands.cpp: ExecuteSnapshot → PrintSnapshotUsage
  → wslservice.exe: WslCoreVm::SnapshotDistribution        → src/windows/service/exe/WslCoreVm.cpp:2693+
       MessageWriter<LX_MINI_INIT_SNAPSHOT_MESSAGE> → Send<...>(message.Span())  (TMessage=结构体! 显式模板参数)
  → init: ProcessSnapshotMessage (main.cpp)                → src/linux/init/main.cpp (LxMiniInitMessageSnapshot 分派)
  → wslplus_snapshot::ParseAction/BuildCommand             → src/linux/init/wslplus_snapshot.cpp (kBtrfsPath=/usr/bin/btrfs)
  → btrfs 完成 → SNAPSHOT_RESPONSE_MESSAGE → Windows 侧
```
消息/类型: lxinitshared.h (LX_MINI_INIT_SNAPSHOT_MESSAGE / _RESPONSE_MESSAGE / ACTION/NameOffset / PRETTY_PRINT)。

## 克隆(B5 唯一化)
```
wsl clone → WSLPlusCommands → WslCoreVm 克隆扩展
 → 首次启动 init: ProcessLaunchInitMessage + LxMiniInitMessageFlagWslplusClone
 → guest 侧唯一化脚本(hostname/ssh host key/machine-id)  → main.cpp:2521 段
```

## 网络附加(AttachMode/额外网卡)
```
wsl network attach → WSLPlusNetworks.cpp → WslCoreVm::ApplyExtraNetworkAdapters
  → EnumerateNetworks → HostComputeEndpoint(Schema2.16/HostComputeNetwork/Policies:PortName)
  → CreateEphemeralHcnEndpoint → ModifyComputeSystem(Add) → guest eth1+
  → guest: StartExtraDhcpClients(VLAN/静态/ACL/QoS/VXLAN: /etc/wslplus-*.conf)  → main.cpp:3660 区
```

## 镜像库
```
wsl image import/list/... → WSLPlusImages.cpp → %USERPROFILE%\.wslplus\images\<name>\{rootfs.tar.gz,manifest.yaml}
```

## USB
```
wsl device ... → WSLPlusUsb.cpp → usbipd 链(宿主) → guest 侧 vhci/协议
```

## 构建/打包链
```
build.yml: configure(SKIP_GLUE=ON, DISABLE_PCH, SETTINGS=true, PACKAGE_VERSION=3.0.0.0[引号!])
 → cmake --build --target msipackage (依赖全链 wsl/wslg/wslservice/.../initramfs)
 → msipackage/CMakeLists.txt → WiX(package.wix.in: SKIPMSIX=1已设, 无 msix Binary)
 → 交付命名 Move-Item → wsl.3.0.0.0.x64.msi → upload-artifact
```

## 常用消息/常量打卡处
- 消息枚举/结构体: src/shared/inc/lxinitshared.h（快照在 1270-1300 区）
- 头文件路径: SocketChannel.h(195=PrettyPrint 调用), message.h(MessageWriter), prettyprintshared.h(PRETTY_PRINT 宏)
- 断言点: WslCoreVm.cpp 1594/2694 区(已修闭环, 别回滚)
