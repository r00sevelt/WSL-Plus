# WSL-Plus vs 官方 WSL（2.9.9.0）差异总表

> 每次上游合并后**更新本表**（合并流程的固定步骤之一）。现状基线上游=`25d3add3`。

## 1. 功能差异（我们多出的）
| 功能 | 官方 | WSL-Plus | 状态 |
|---|---|---|---|
| 磁盘布局 | ext4（VHDX 内） | **btrfs + @ 子卷 + set-default**（快照/克隆/COW 基础） | ✅ 已实现（仅全新盘生效） |
| 快照 create/list/restore/delete | ❌ | ✅（guest wslplus_snapshot + Windows 命令面 + 协议 action/name 解耦） | ✅ 冒烟过 |
| 克隆（含唯一化 B5） | ❌ | ✅（首次启动 hostname/ssh key/machine-id 唯一化） | 代码在 |
| 企业网络 | NAT/桥接基础 | ✅ 额外网卡(AttachMode/HCN 动态端点) + VLAN + VXLAN(v0.1) + ACL(nft/iptables) + QoS(tc) + 静态地址 | 代码在, 未全测 |
| USB 直通 | 手动 usbipd | ✅ WSLPlusUsb 封装链 | 代码在, 未全测 |
| 镜像库 | 一次性 --import | ✅ import/list 集合管理 | 代码在 |
| 管理 GUI | WSL Settings(WinUI, 桌面) | **Qt 面板(独立)**——路线图 P1 | 🔴 骨架未建 |
| 设置面板策略 | 官方为唯一 | 官方 Settings 常驻作默认入口 + Qt 共存 → V1 后接管协议 | 规划中 |

## 2. 命令面差异
- **官方 `--xxx` 全兼容**（叠加式策略, ADR#3 修订——P0 恢复项）
- **扩展子命令**：`wsl snapshot/…`、`wsl network/…`、`wsl image/…`、`wsl device/…`（WSLPlusCommands 分派, 宽窄边界层在此）
- 帮助总表展示扩展命令 = P0 待实现项（现仅 PrintSnapshotUsage）

## 3. 版本/升级差异
- 升级码 **同官方**（6D5B792B-…）→ `wsl.3.0.0.0.x64.msi` 覆盖安装官方版, 数据/发行版保留
- 版本线: 3.0.0.0（>官方 2.9.9.0; 规则=发布前 gh api 复核, ≥我们→大版本+1）
- 官方 2.9.9.0 vs 我们 345.2MB: 构成=官方同类盘子(WSLg 主分母同源) ± (我们的扩展几MB − msix 内嵌几MB)

## 4. 分发差异
| | 官方 | WSL-Plus |
|---|---|---|
| 安装包 | MSI（内嵌 gluepackage.msix）+ Store MSIXBUNDLE | **单 MSI**（MSIX 根除: 无内嵌, SKIPMSIX=1 官方链跳过） |
| 更新通道 | Store/官方管线 | 自持 GitHub Actions（1h/轮） |
| 构建 | 微软内部 ADO | 公开仓库 Actions（排队易取消→dispatch 重跑最稳） |

## 5. 部署差异（用户感知层）
- 老发行版（ext4）**不自动转 btrfs**——快照仅对新建盘生效
- 官方 `.wsl`/tar rootfs **直接兼容**（btrfs 在磁盘层, 与 rootfs 解耦——无需自建 rootfs）
- 快照命令依赖 rootfs `/usr/bin/btrfs`（btrfs-progs）——精简镜像缺失时 apt 装（BACKLOG B6）

## 6. 工程质量差异（坦诚）
- 验证深度: 官方=测试矩阵/flight/遥测; 我们=B1-B6 冒烟为主（账本: docs/BACKLOG-DEFECT.md）
- 本地化: 官方 22 语言; 我们=英文为主
- 更新节奏: 官方并行发布; 我们=上游合并例行化（4 次零冲突——机制见 §8.4/ADR#6）

## 维护规则
每次 `git fetch origin` 有新提交 → 合并后**逐行核对本表**（功能面是否被上游覆盖→更新状态）→ 再更新 CHANGELOG。接任者第一周任务之一=把本表过一遍。
