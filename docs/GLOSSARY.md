# 术语表（WSL-Plus 语境快速索引）

## 平台/微软侧
| 词 | 含义 |
|---|---|
| HCS | Host Compute System（微软开放 VM 管理 schema API——WSL/Docker 用） |
| WHv/WHPX | Windows Hypervisor Platform（第三方 VMM 借 Hyper-V 的用户态 API） |
| VMP | Virtual Machine Platform（WSL2 用的 Hyper-V 子集特性） |
| VMBus | Hyper-V 半虚拟化总线（宿-guest 通信） |
| HCN | Host Compute Network（HNS 网络栈——端点/网络管理） |
| wslservice.exe | WSL 宿主动态服务（WslCoreVm 所在——我们的扩展主战场） |
| init (src/linux/init) | WSL guest 初始化程序（ELF，交叉编译；我们的快照/网络 guest 逻辑在此） |
| initrd.img | CPIO 归档（内容=init 单文件+E5 注入的工具——create-initrd.ps1） |

## WSL-Plus 特有
| 词 | 含义 |
|---|---|
| @ 子卷 | btrfs 根系统子卷（mkfs 后 `btrfs subvolume create /@` + set-default——挂载默认=@） |
| /@snap-<name> | 快照子卷（@ 的兄弟）——`wsl snapshot create` 后即此 |
| E5 | 静态工具注入（create-initrd.ps1 ExtraToolsDir——btrfs-progs 进 initrd，避开 rootfs 依赖…部分） |
| AttachMode | 额外网络适配器机制（WslCoreVm::ApplyExtraNetworkAdapters——HCN 动态端点+ModifyComputeSystem(Add)） |
| 替换式→叠加式 | 命令面策略修订（ADR#3）——官方 --xxx 全兼容 + 扩展子命令并存 |
| wslplus_snapshot | guest 快照模块（ParseAction/BuildCommand/DefaultSnapshotName——kBtrfsPath=/usr/bin/btrfs） |
| networks.yaml | WSL-Plus 网络配置 schema（.wslplus/…——NETWORK-PHASE-DESIGN） |
| 镜像库 | %USERPROFILE%\.wslplus\images\（rootfs.tar.gz+manifest.yaml——WSLPlusImages） |
| SKIPMSIX=1 | wix 属性：官方 MSIX 动作链整体跳过（我们没有内嵌 MSIX，必须开着） |
| c_networkAdapterPrefix | ResourcePath 前缀（HCN 网络适配器） |
| Swap 配置 | 见 wslplus 文档 design 文件（C9-OVN-DESIGN=OVN 控制面 v2 愿景） |

## 易混（坑）
- HostComputeEndpoint（创建入参/设置） vs HNSEndpoint（查询结果） vs EphemeralHcnEndpoint（创建返回, .Id 小写, 无 MacAddress）
- CiS 条件 `"true"` 字面量 vs CMake ON
- PACKAGE_VERSION 传参引号（pwsh 拆 .0.0.0）
