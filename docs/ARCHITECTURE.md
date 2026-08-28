# 架构与端到端数据流（心智模型——人/AI 先读这份再读代码）

## 分层总览
```
[用户层]   wsl.exe(命令面: 官方兼容+WSL-Plus 子命令) │ Qt 面板(未来, 独立)
    │ wslservice.exe (Windows 服务, 主宿主)
[宿主层]   │  ├─ WslCoreVm.cpp (VM 生命周期/配置/虚拟机交互——4 个扩展点)
    │  ├─ WSLPlus*.cpp (业务命令: Commands/Networks/Usb/Images/Devices)
    │  └─ WSL202 HCS → Hyper-V/VMP(微软宿主, 闭源, 我们只调客户接口)
    ↓ VMBus/HCS 通道
[guest 层] src/linux/init/main.cpp (init, ELF; EX) → wslplus_snapshot.cpp (快照模块)
    ↓ rootfs = btrfs @ 子卷(我们定制的磁盘布局)
[数据层]   发行版 VHDX(内核自行挂载btrfs) / 镜像库 %USERPROFILE%\.wslplus\images
```

## 端到端数据流：快照（最能代表设计哲学）
```
wsl snapshot create test1
 → wsl.exe(WSLPlusCommands::ExecuteSnapshot, 宽窄边界层)
 → wslservice.exe(WslCoreVm::SnapshotDistribution)
     ├─ 构造 LX_MINI_INIT_SNAPSHOT_MESSAGE(MessageWriter, Send<T>(Span()))
     └─ miniInitChannel → vsock → init
 → src/linux/init/main.cpp ProcessSnapshotMessage(分派: LxMiniInitMessageSnapshot)
     └─ fork → wslplus_snapshot::ParseAction → BuildCommand
        ("/usr/bin/btrfs subvolume snapshot / /@snap-test1" ← kBtrfsPath)
 → btrfs(done): return code 经 Response 消息 → Windows 侧收尾
```
**注意**: 命令构造在 guest 侧;Windows 侧只传 action+name(协议解耦——c 与 guest 可独立测试)。

## 网络扩展链路(AttachMode)
```
wsl network 附加 → WSLPlusNetworks.cpp → WslCoreVm::ApplyExtraNetworkAdapters
 → EnumerateNetworks 找 vSwitch → HostComputeEndpoint(Schema 2.16+HostComputeNetwork+PortName policy)
 → CreateEphemeralHcnEndpoint(返回 EphemeralHcnEndpoint, .Id)
 → ModifyComputeSystem(Add, NetworkAdapter) → guest 侧 eth1+ 枚举
 → guest: StartExtraDhcpClients + 配置(VLAN/静态/ACL/QoS/VXLAN 文件 /etc/wslplus-*.conf)
```

## 我们的"修改点"为什么永远不冲突（§9 哲学）
- 新增: WSLPlus*.cpp/h、wslplus_snapshot.*、qtgui、docs——**独立文件,上游永不触碰**
- 修改: 仅 §9 清单(CMakeLists 开关/WslCoreVm 扩展/main.cpp 段/wix.in 2 处/LxssUserSession 等)——每次上游 merge 后检查这份清单即可

## 时间/体积事实
- 构建≈1h(windows-latest + msipackage 全链 + WiX); 产物 345MB(=官方 344.3 同级: WSLg 大盘子同源, 我们的增量≈几 MB, msix 内嵌≈几 MB, 互抵)
