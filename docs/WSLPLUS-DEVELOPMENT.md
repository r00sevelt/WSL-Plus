# WSL-Plus 开发手册（企业级完整版 v0.3）

> 版本: v0.3（2026-08-25）| 维护: Kex 项目 | 质量声明: 本手册与代码同步（API 签名逐字取自源码头文件）
> 目标读者: 任何新开发者 / AI 智能体（读懂本手册即可无监督接手开发与排障）
> 读者入口: 先读 §1-2 → §8（构建）→ 依任务索引 §4

## 目录（导航）
1 产品概述 · 2 架构(2.1分层 2.2 ADR 2.3端到端) · 3 部署 · 4 功能手册 · 5 配置 · 6 CLI(6.4 迁移对照)
· 7 API参考(7.7 错误约定) · 8 构建CI(8.1环境 8.2坑库 8.3预算 8.4上游合并) · 9 源码地图 ·
10 测试矩阵 · 11 故障(11.5 限制性能) · 12 安全 · 13 路线 · 14 Doxygen · 15 术语 · 16 交接

## 变更记录
| 版本 | 日期 | 变更 |
|---|---|---|
| v0.1 | 08-25 | 初版 12 章 |
| v0.2 | 08-25 | 企业级补全: API全参考/端到端时序/坑证据库/环境步骤/源码地图/测试矩阵/术语/交接表/Doxygen |

---

## 1. 产品概述
| 项 | 值 |
|---|---|
| 名称 | WSL-Plus（底层=Windows Subsystem Linux；Plus=增强分署；Max=远期完全体） |
| 定位 | 微软 WSL 之上重构增强的**虚拟化平台**：btrfs 快照/克隆、企业级网络拓扑、外设共享、镜像库、图形管理 |
| 技术路线 | 不改 hypervisor/不 fork 内核；全部改动在 WSLService 应用层 + guest init + 模块扩展 |
| 许可 | WSL 主体 MIT（本仓库）、WSL2 内核 GPL-2.0（另行分发）；本仓库私有 |
| 分发 | 仅 exe + msi（MSIX 家族根除：BUILD_BUNDLE 未启用 + WSL_SKIP_GLUE_PACKAGE=ON） |
| 安全语义 | 设备 eject=归还（绝不删硬件/驱动）；rm=仅清登记 |

---

## 2. 架构（分层 + 端到端链）

### 2.1 分层图
```
[用户层] wsl.exe(替换式命令集) + wslplus-desktop(Qt,独立)
   ↕ cmdline
[CLI 服务桥] WSLPlusCommands(分发/Narrow-Widen 边界) → SvcComm(RPC) → ILxssUserSession(COM)
   ↕
[服务路由] LxssUserSessionImpl(注册项→utility VM 选择/路由)
   ↕
[VM 执行] WslCoreVm: VHD 生命周期/HCS 消息/网络引擎/HNS 端点/串口 ComPort/快照消息
   ↕
[平台(微软,不改)] HCS · HNS/VMP · Microsoft Hypervisor
   ↕
[guest(Linux 改动)] init(mini-init): mkfs.btrfs/@子卷/挂载/网络引擎 + wslplus_snapshot
   ↕
[数据层] %USERPROFILE%\.wslplus\*（networks/devices/images）+ /etc/wslplus-*.conf（guest）
```
**模块归属**（本次新增全部)：
- Windows common: `WSLPlusCommands.h/.cpp` `WSLPlusNetworks.*` `WSLPlusDevices.*` `WSLPlusImages.*` `WSLPlusUsb.*`
- Linux init: `wslplus_snapshot.h/.cpp`
- GUI: `src/windows/qtgui/{CMakeLists,main.cpp}`（独立 Qt6 子工程, 不依赖 common）
- 服务端扩展：WslCoreVm(4 方法) + LxssUserSession(5 接口) + svccomm + wslservice.idl

### 2.2 架构决定（ADR 全表）
| # | 决定 | 理由/替代否决 |
|---|---|---|
| 1 | btrfs 根盘(替代 ext4) | WSL 内核原生 btrfs(m)；子卷快照/COW；zfs=许可+内核无→否；xfs=无快照→否 |
| 2 | 回滚=默认子卷切换(set-default) | 在线不能换挂载；重启切根+保底 pre-restore 快照（"只进不退"） |
| 3 | 替换式命令集 | Incus 语义；PS 别名免疫；旧标志参数移除 |
| 4 | 多vNIC=HNS 端点动态 attach | 与微软引擎同构；guest virtio-net 自动枚举；VM 静态 HCS JSON 无网卡字段 |
| 5 | 设备共享优先+三档(auto/manual/dedicated) | VMware/Parallels 同构；硬件物理独占→"共享=可移动归属"；eject=归还 |
| 6 | VLAN/ACL/QoS/静态/VXLAN 走 guest Linux 原生 | 免 Windows 侧传递通道；HNS wrapper 无 VLAN 字段；nft/tc/ip 为产品级 |
| 7 | 后端可换(IUsbBackend: usbipd v1/usbredir v2) | QEMU/SPICE 多协议哲学；usbipd=微软官方通道 |
| 8 | 上游双轨(master 微软/ dev 我们 / feature* 开发零云) | 零冲突合并；预算纪律（云端动作用户指令） |

### 2.3 四条端到端时序链（供调试导航）
```
[快照] CLI → SvcComm::SnapshotDistribution → impl(attach VHD→lun) → WslCoreVm
        → MiniInit 消息(Snapshot) → guest ProcessSnapshotMessage → wslplus_snapshot::BuildCommand
        → btrfs subvolume … → 响应码回传; list 输出经 OutputHandle 管道回终端
[克隆] CLI(--full) → CloneDistribution → impl: CreateLinkedVhd/COPY → 注册项 Create(CLONE flag)
        → 下次启动 guest 唯一化(0x10→0x40 flag→/etc/wslplus/unique.done 幂等)
[多网卡] network attach(net=bridge) → attach/Attachments → VM 启动 ConfigureNetworking
        → ApplyExtraNetworkAdapters: HCN 枚举 Open/Endpoint → ModifyComputeSystem(Add)
        → guest 出现 eth1+ → StartExtraDhcpClients(eth1+; 支持 vlan 子接口)
[串口] device attach serial → AttachSerialPort → impl → WslCoreVm::AttachSerialPort:
        CreateNamedPipe + Modify(Add ComPort) + 桥线程(COM↔pipe 115200) → guest /dev/ttyS1
        eject → 桥停 + Modify(Remove) → COM 归还
```

---

## 3. 系统要求与部署
### 3.1 环境（从零）
- Windows 11（22H2+）；启用"虚拟机平台/Windows Subsystem for Linux"组件（WSL2 基础）
- VBS/HVCI: 可关（不影响；本产品不经 MH 深度）
- 安装: `msiexec /i wsl.msi`（交互有系统向导 UI；/q 静默）
- 覆盖: 同官方 UpgradeCode→直接升级（版本号必须 ≥当前安装——发布前把 PACKAGE_VERSION 设高位，见 §8.3）
- Store 用户: `wsl --uninstall` 后装 MSI
- 数据保留: 发行版 VHDX(%USERPROFILE%/...) + 注册配置不受卸载影响
- 回滚: 卸载→装官方；详见 docs/PUBLISHING.md（§16）

---

## 4. 功能手册（40+11 全部实现）

### 4.1 快照/克隆
```
wsl snapshot create mybox [名字]    # btrfs 子卷快照 /@snap-<名>（缺省 auto-<pid>）
wsl snapshot list mybox            # 输出=btrfs subvolume list（经管道）
wsl snapshot restore mybox <名>    # 保底(pre-restore-<名>) + set-default(重启生效)
wsl snapshot delete mybox <名>
wsl clone mybox newbox [--full]    # 链接(COW VERSION_3) / --full 独立副本
# 唯一化: 克隆实例首次启动自动重造 hostname/SSH 主机密钥/machine-id（幂等）
```

### 4.2 网络（企业级） 命令+四配置
```
wsl network ls|show|add|rm|attach|detach|ports-on
  add 参数: --type nat|bridge|host-only  --cidr 192.168.x.0/24  --vlan N
  ports-on <inst>: 读 attached networks ports → JSON → ApplyPortMappings → 监听+Relay 转发
guest 配置文件(重启后自动生效):
  /etc/wslplus-static.conf  "eth1 192.168.10.10/24 192.168.10.1"
  /etc/wslplus-vlan.conf    "eth1 100"        # ip link add eth1.100 type vlan + DHCP
  /etc/wslplus-acl.conf     "drop tcp 0.0.0.0/0 22"   # nft(缺省 iptables)
  /etc/wslplus-qos.conf     "eth1 100"        # tc tbf rate 100mbit
  /etc/wslplus-tunnel.conf  "eth1 100 10.9.9.1" # vxlan id 100 remote (OVN 数据面)
边界: Wi-Fi 桥接=平台不支持(硬红线)；OVN 控制面=v2
```

### 4.3 设备
```
wsl device list|add <id> <usb|serial|parallel> <host> [--instance x --policy auto|manual|dedicated]|rm|auto|attach|eject|watch
  attach serial → 重定向真挂载；parallel → USB-LPT 桥(usb 后端)；eject → 归还(优雅释放)
  watch → 热插拔守护(5s diff→auto 规则自动 attach)  —— Ctrl+C 停
```

### 4.4 镜像库
```
wsl image list|import <tar> <name> [--desc x]|rm <name>
  库=%USERPROFILE%\.wslplus\images\<name>\{rootfs.tar.gz,manifest.yaml}
  官方 .wsl tar 兼容(根内容解包与文件系统无关)
```

### 4.5 GUI（N0-GUI 骨架，独立 Qt6）
- 运行: 编译 qtgui 后 wslplus-desktop.exe；页签: 发行版/网络/设备
- 后端通信=调用 wsl.exe 命令面（零耦合）；后续: QML 拓扑画布(=消费 networks.yaml 渲染 VMware 式编辑器)

---

## 5. 配置参考
### 5.1 networks.yaml 全 schema + 样例
```yaml
networks:
  - name: web            # 必填, 唯一
    type: nat            # nat|bridge|host-only
    cidr: 192.168.100.0/24
    dns: 192.168.100.1
    vlan: 100            # 可选(vlan 子接口用)
    ports:
      - host-ip: 0.0.0.0 # 可选
        host-port: 8080
        guest-port: 80
attachments:
  mybox: [web]           # 实例→网络绑定(4a)
```
### 5.2 devices.yaml
```yaml
devices:
  - id: my-serial
    type: serial         # usb|serial|parallel
    host: COM3
    instance: mybox
    policy: auto         # auto|manual|dedicated
```
### 5.3 镜像 manifest.yaml
```yaml
name: mybox-base
description: ""
architecture: x64
base-version: ""
created-at: 1780000000   # Unix 时间戳(uint32)
```

---

## 6. CLI 参考
### 6.1 命令总表（替换式命令集）
命令均 `wsl <子命令>`；帮助: `<子命令>`后不带参数即打用法。
| 组 | 子命令 | 参数(示例) |
|---|---|---|
| snapshot | create/list/restore/delete | `mybox [name]` |
| clone | [--full] | `src dst` |
| network | ls/show/add/rm/attach/detach/ports-on | 见 §4.2 |
| device | list/add/rm/auto/attach/eject/watch | 见 §4.3 |
| image | list/import/rm | 见 §4.4 |
### 6.2 PowerShell/cmd 规范（蓝图 §5.7 生效中）
- 一律前缀调用（免疫别名表）；含 `$ * |` 整串单引号；`--%` stop-parsing；cmd 脚本 `%%`
### 6.3 退出码约定
- 0=成功；-1=使用/配置错误（与 wsl.exe 惯例一致）；服务端错误经 GetSystemErrorString 打印

### 6.4 官方 WSL → WSL-Plus 命令迁移对照（旧命令与新版对应）
| 旧(wsl --xxx) | 新(替换式) |
|---|---|
| `--list / -l` | `wsl ls` |
| `--launch <distro>` | `wsl start <distro>` |
| `--terminate / --shutdown` | `wsl stop <distro>` |
| `--import` / `--export` | `wsl image import`（库）/ 保留在底层（备份语义仍可用） |
| `--set-default` | 归入 `wsl config` 方向（v0.2+ 语义化） |
（旧标志参数在替换式命令集下不再维护——已按 ADR#3 移除；迁移提示=使用 `wsl <子命令>` + `wsl <子命令>` 空参数看帮助）

---

## 7. API 参考（权威：签名取自头文件,与代码逐字一致）
### 7.1 WSLPlusNetworks（.wslplus\networks.yaml）
| 函数 | 签名 | 语义 |
|---|---|---|
| ConfigPath | `std::filesystem::path ConfigPath()` | 配置路径 |
| Load | `std::vector<NetworkConfig> Load()` | 全量读（无文件=空；解析失败 throw） |
| Save | `void Save(const NetworkConfig&)` | 同 name 覆盖；Validate 前置 |
| Remove | `void Remove(const std::string&)` | 未找到 throw E_INVALIDARG |
| Attachments | `std::vector<std::string> Attachments(const std::string& instance)` | 实例网络列表 |
| Attach / Detach | `void Attach/Detach(instance, network)` | 幂等/解绑；持久 |
| Validate | `void Validate(const NetworkConfig&)` | 名称非空/type 三值/端口 1-65535 |
（结构: NetworkConfig{name,type∈{nat,bridge,host-only},cidr,dns,vlan?,ports[]}；
 NetworkPortMapping{hostIp,hostPort,guestPort}）
### 7.2 WSLPlusDevices
| 函数 | 语义 |
|---|---|
| Load/Save/Remove/SetPolicy(id,Policy,instance)/Validate | 同网络模块模式；Validate: id+host 非空/type 三值 |
### 7.3 WSLPlusImages
| 函数 | 语义 |
|---|---|
| ImagesRoot() | ~\.wslplus\images |
| List()/Read(name)/Import(tar,name,desc?) | 库扫描/读清单（不存在 throw ERR_FILE_NOT_FOUND）/拷贝+生成 manifest（Validate 要求 createdAt≠0） |
| Remove(name)/Validate(manifest) | 删目录/检查 name+arch+时间戳 |
### 7.4 WSLPlusUsb（后端抽象）
| 接口 | 方法 |
|---|---|
| IUsbBackend | `BackendKind Kind() const noexcept; std::vector<std::string> Enumerate() const; void Attach(busId, const GUID& vmId); void Detach(busId); bool Available() const noexcept` |
| CreateDefault() | usbipd-win 进程封装（list/attach/detach；错误→WSL_LOG+E_FAIL） |
| CreateUsbredir() | 接口壳（Available=false; 其余 E_NOTIMPL），见 docs/USBDEV-STUDY.md |
| WatchLoop(backend, intervalSec, callback) | 轮询 diff→onNewDevice(busId)（false=终止） |
### 7.5 Linux wslplus_snapshot
| 函数 | 语义 |
|---|---|
| `Action ParseAction(const std::string&)` | create/list/delete/restore→enum；未知→Unknown |
| `std::string BuildCommand(Action, const std::string& name)` | 生成待执行 btrfs/touch 命令（常量表: kBtrfsPath=/usr/bin/btrfs; kSnapshotPrefix=/@snap-；restore=保底+set-default 链） |
| DefaultSnapshotName / RestoreMarkerPath / BuildSetDefaultCommand | auto-<pid> / 标记路径 / `ID=$(btrfs subvolume show '…'|awk); set-default $ID /` |
### 7.6 服务端扩展（接口）
- WslCoreVm: `SnapshotDistribution(Lun, HANDLE output, action, name)` / `AttachSerialPort(LPCWSTR com)` / `DetachSerialPort()` / `ApplyExtraNetworkAdapters()`
- ILxssUserSession(idl): `SnapshotDistribution` `CloneDistribution` `ApplyPortMappings` `AttachSerialPort` `DetachSerialPort`
- SvcComm: 同上五方法（客户端封装）
- 安全注意: 所有扩展方法均走原版 IFACEMETHOD 范式——`ServiceExecutionContext context(Error)` + `const auto session = m_session.lock(); RETURN_HR_IF(RPC_E_DISCONNECTED, !session);`（→服务端实现层在 m_instanceLock 内）——任何新增接口必须复用此模式,不得臆造辅助函数
- 错误经 CATCH_RETURN(Error) 宏（ServiceExecutionContext 写入 Error 出参）

---

## 8. 构建与 CI（含环境逐字与坑证据库）
### 8.1 环境搭建（逐步）
1. 安装 VS 2026 Enterprise（组件: MSVC v180、ATL、**VC.Llvm.Clang(22.1.3)**、**x64/x86 Spectre 缓解库**、ATL Spectre）
2. CMake ≥3.25（推荐 4.4.x）；Git for Windows；开发者模式开启
3. 仓库克隆至**全英文路径**（中文路径→GBK 写入 PCH→C4828/MSB8084——坑#3）
4. configure（唯一配方）:
   `cmake . -A x64 -DWSL_SKIP_GLUE_PACKAGE=ON -DWSL_DISABLE_PCH=ON -DCMAKE_BUILD_TYPE=Release`
5. 构建: `cmake --build . --config Release --target wsl wslservice -- /m:4`（云满配）
6. Qt GUI(独立): `cmake -S src/windows/qtgui -B build-gui -DCMAKE_PREFIX_PATH=<Qt6>`（需 Qt6; Doxygen 可选）

### 8.2 坑证据库（已修 10 坑 + 未决 2 坑; 每坑: 症状/根因/修复/验证）
| # | 症状(日志证据) | 根因 | 修复 | 验证 |
|---|---|---|---|---|
| 1 | MSB8013 无 Release\|x64 | 未 -A x64 | configure 加 -A x64 | configure 后 vcxproj 含 x64 配置 |
| 2 | C4819 代码页936 | 中文系统+源 UTF-8 | /utf-8(CMakeLists CXX_FLAGS) | 重编无 C4819 |
| 3 | MSB8084 JsonRpc+ C4828 @pch | 仓库中文路径→GBK 写入 PCH | 全英文路径 | 移动后不再现 |
| 4 | C3859/C1076 PCH 虚拟内存 | 大 PCH /Zm 误配 | WSL_DISABLE_PCH+全局 /FI precomp.h | 禁 PCH 后内存平稳 |
| 5 | C2653 wil 类未识 | DnsResolver 先包含未含 precomp | /FI 注入(同#4) | 编译过 |
| 6 | C2679 宽窄混传 | CLI 裸 argv/LPSTR→窄接口 | Narrow/Widen 助手(边界转换) | 全 CLI 编译过 |
| 7 | C2065 FOLDERID_UserProfile | 头链死坑(shlobj_core 无效) | GetUserProfileDirectoryW(userenv) | 编译过 |
| 8 | C2317/C2311 try/catch | 插入残留孤立 CATCH_RETURN() | 删孤行 | 编译过 |
| 9 | C2440 ModifyRequestType 域混用 | hns::ModifyRequestType 传给 hcs 域函数 | 一律 hcs::ModifyRequestType(Add/Remove 限定) | 编译过(第11轮) |
| 10 | C3083/C2039 臆造命名空间 | wslplus::networks::Attachments::Load 引用错层 | include WSLPlusNetworks.h | 编译过(第11轮) |
| 11 | C2664 schema 错位 | HNSEndpoint(查询schema) 作创建入参; 实要 HostComputeEndpoint(设置schema) | 照 BridgedNetworking.cpp:50-63(SchemaVersion 2.16/HostComputeNetwork/Policies) | ⚠️ 未决(WslCoreVm.cpp:1594) |
| 12 | C2664 Send 模板绑定 | Send(message.Span()) 临时量绑非const引用 | Send(message) —— 对齐 WslCoreVm.cpp:2338/2670/2723 模式 | ⚠️ 未决(WslCoreVm.cpp:2694) |
（坑#1-10 已修闭环; #11-12 待修——以当日云端失败日志为准）

### 8.3 CI 与预算
- GitHub Actions(公开仓/windows-latest 4C16G): `wsl+wslservice /m:4` 全量；concurrency cancel-in-progress；默认分支 dev
- 计费: 公开仓免费额度充裕（若日后转私有仓: 2000 分钟/月, Windows ×2 计费=实 1000）；备胎: **Azure Pipelines（1800/月 1:1）**（WSL 官方 CI 同源）
- 云端纪律: 推送/合并/取消/检查一律=用户指令（开发准则红线）

### 8.4 上游合并流程（双轨三步——新人必读）
```
① fetch origin（微软更新到达后）
② git checkout master && git merge --ff-only origin/master   # master 干净=零冲突
③ git checkout dev && git merge master                        # 冲突只在此（我们的修改区
   与上游本就不重叠——若真冲突=以 WSL-Plus 语义为基准解决）
推送纪律: 合并→示用户→经指示才 push（云动作用户指令）
```

### 8.4a 上游同步注意事项（防踩坑清单——必读）
```
【远程语义（双 remote,别搞混）】
  origin = 微软上游（fetch/pull 用这个）     plus = 我们公开仓（push 用）
  ⚠️ 坑#1（我们踩过）: master 分支的 upstream 曾因 push -u plus 变成 plus/master
     → git pull 在 master 上会拉"我们自己"（空更新,白跑）——
     正确: git merge --ff-only origin/master（显式 origin）
【顺序铁律】
  dev 永不直接 pull/merge origin（上游只经 master 进；保持 master=上游、dev=我们）
【覆盖保护（我们改动零丢失的保障）】
  所有我们的改动=独立文件(wslplus_*/docs/*) + 有限修改点(§9 清单)
  → merge 时上游碰撞概率≈0；即使冲突: git 停下等你,不会自动覆盖
  危险命令禁用: reset --hard / checkout --（除非明确要丢弃——不用于上游同步）
  反悔: git merge --abort（上游合并前一步即回原点,零损失）
【推送前后】
  推送前: git status 干净?（先 commit 自己的改动再走①②③——别带脏状态合并）
  推送 = 用户指令（云纪律——文档 commit 只进本地,合并轮时随代码一起推）
【上游带进来的"非代码"】
  上游 .github/*（workflows/dependabot.yml）会随 merge 进入——它们会触发我们的
  Actions（影响预算）: 已禁用微软 7 个 workflow; dependabot.yml 如再更新需复查
  （PR 是 dependabot 自动开——无碍,可无视/禁用,见 docs/components/README）
```

### 8.5 版本号策略（发布红线）
- PACKAGE_VERSION 用于 MSI 升级判定；**必须 ≥ 已装任何版本**（官方或 Plus）
- 建议: 产品版=主.次.补丁.0（如 3.0.0.0）；构建 fallback（无 tag 时）自动 1.0.0.0 仅编译期用——**发布必须显式传版本**
- 改法: configure 时 `cmake -DPACKAGE_VERSION=3.0.0.0 ...`（或改 version_functions.ps1 的 fallback 常量）

---

## 9. 源码地图（新老文件分离）
```
仓库新增(WSL-Plus 专属——上游 merge 零冲突的保证):
  src/windows/common/WSLPlus*.h/.cpp(5模块) + WSLPlusUsb
  src/windows/qtgui/*(独立 Qt 子工程)
  src/linux/init/wslplus_snapshot.* (guest 快照模块)
  docs/WSLPLUS-DEV-GUIDELINES * NETWORK-PHASE-DESIGN * PUBLISHING * USBDEV-STUDY * C9-OVN-DESIGN
修改点(有限): CMakeLists(开关/链接) · WslCoreVm(4扩展) · WslCoreFilesystem(CreateLinkedVhd)
  · LxssUserSession(5接口) · svccomm/* · wslservice.idl · main.cpp(FormatDevice/网络/快照处理)
  · lxinitshared.h(消息) · tools/create-initrd.ps1(E5)
阅读入口: §2.3 时序链 → 对应文件 grep 函数名即可上手
```

---

## 10. 测试矩阵（每功能验收断言——E7 用）
| ID | 功能 | 验收断言 |
|---|---|---|
| T1 | 快照 create/list/restore/delete | 建→改坏→restore 重启→文件复原; list 文本含条目 |
| T2 | 克隆(链接) | 秒级;< dst 能启动; 唯一化(hostname 不同) |
| T3 | 多网卡/VLAN | guest ip link 见 eth0+eth1; vlan conf 后 eth1.100 up+DHCP IP |
| T4 | 端口映射 | ports-on 后 host:port 可达 guest 服务; eject/释放 |
| T5 | 串口 | attach COM3→guest ttyS1 echo; eject→COM 回宿主 |
| T6 | USB | (需 usbipd 环境) watch 插入→auto attach; eject 归还 |
| T7 | 镜像 | import 官方 tar→list 显示; rm 干净 |
| T8 | 桥接/静态 | static conf→iface 拿指定 CIDR; 网关路由 |
| T9 | ACL/QoS/隧道 | acl drop 生效; qos 限速观测; vxlan 对端互通 |

---

## 11. 故障排除速查
运行时问题→按症状查清单: 快照无效果(缺 btrfs-progs→E5 静态注入) · 网卡不出现(guest 配置→syslog) · 设备 eject 未归还(桥线程/通道检查) · 安装失败(版本号低→§8.5 版本红线)

### 11.5 限制与性能（诚实边界）
| 项 | 边界 |
|---|---|
| Wi-Fi 同网段桥接 | ❌ Windows 平台不支持（红线,详见 ADR#6） |
| MCP/QEMU | 不需要（后端预留给 usbredir/libUSB v2；QEMU 线=独立 UHP 项目） |
| 快照粒度 | btrfs 子卷级（整根）——文件级可后续 v0.2 |
| 并口 | 仅 USB-LPT 桥（原生 ISA LPT 不支持） |
| OVN | 数据面(vxlan)已实现；控制面=v2（见 C9-OVN-DESIGN.md） |
| GUI | Qt 骨架（编译需 Qt6 环境；首版页签为命令面对话） |
| 上下文预算 | DeepSeek 1M 模型窗——工程已用"文件持久+注入"扩容（见手册导语/开发准则） |

## 12. 安全与合规
MIT/GPL 遵守; 私有不发布; eject 语义; usbipd 外部组件(安装需用户授权——属于安装期许可项)

## 13. 路线图
v0.1: 首编全量绿（进行中）→ v1 发布(版本红线/验收 T1-T9) → v0.2: usbredir/libUSB 后端 / OVN 控制面 / GUI 拓扑 / 静态工具拉包 → 远期: UHP 线 / WSL-Max

## 14. Doxygen（B件套说明）
源码为 Doxygen 风格注释；发布时在根目录运行 `doxygen Doxyfile`（配置: INPUT=src/windows/common src/linux/init; EXTRACT_ALL=YES; GENERATE_HTML=YES）→ 全接口自动文档=永远与代码同步

## 15. 术语表
VHDX 差异盘=COW链接克隆 | MiniInit=微软 guest 启动协议 | HNS/HCN=微软虚拟网络栈 | ttySx=Linux 串口设备 | VXLAN=隧道数据面 | OVN=逻辑网络控制面 | 三档策略=auto(自动共享)/manual(手动)/dedicated(独占直通)

## 16. 文档索引与交接检查表
```
核心: 本手册 | 资产管理\WSL-Plus\: 任务清单(40+11) / 审计日志 / 参考资料索引
仓库docs/: GUIDELINES / NETWORK-PHASE-DESIGN / PUBLISHING / USBDEV-STUDY / C9-OVN-DESIGN
蓝图: 架构图\wsl-incus-blueprint-v2.html（CLI/shell 规范来源）
交接检查(12条): ①能 configure(§8.1) ②能全量编译 ③能跑 4 条端到端链路(§2.3)
  ④能用全部 6 组 CLI 命令 ⑤会排雷(§8.2 表) ⑥理解 ADR(§2.2 三分钟后口述)
  ⑦知道设备安全语义(eject) ⑧知道上游双轨 ⑨知道云时刻纪律 ⑩会更新本手册
  ⑪路径习惯(全英文) ⑫版本红线(发布前)
```
