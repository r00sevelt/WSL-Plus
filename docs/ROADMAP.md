# WSL-Plus 路线图与缺口清单（交接必读）

> 结论: 引擎级能力全部完成(22 轮验证, 345MB 单包可装); **未完成=产品化层(GUI/兼容/品牌)**。本文档=下一位开发者的工作排序依据。

## 现状一页（2026-08-28）
- 交付物: `wsl.3.0.0.0.x64.msi`（345MB, 全链+WSL Settings+MSIX 根除+SKIPMSIX 修复）
- 架构: 官方 WSL 2.9.x 基底(25d3add3 合并, 4 次零冲突) + WSL-Plus 扩展(快照/克隆/网络栈/USB/镜像/静态 btrfs)
- 构建: GitHub Actions `msipackage` 全链≈1h; **push 触发易被排队取消——用 `gh workflow run ... --ref dev`(dispatch) 最稳**
- 系统身份: 同官方 UpgradeCode/文件/服务名(覆盖式安装); 售前品牌=待定(见 ROADMAP §3)

## P0（发布级, 建议第一周）
1. **命令面回退"叠加式"**（第 23 轮规划）: 官方 `--xxx` 全兼容恢复 + 扩展子命令(wsl snapshot/…)并存——**消除"命令残缺"**
2. **顶层帮助总表**: wsl --help 展示 WSL-Plus 扩展命令表(现只有 PrintSnapshotUsage)
3. **品牌显名**: 候选 UHP-WSL(推荐)/WSLHarbor——改 wix Package 显示名+开始菜单+文档题; 系统身份不动

## P1（产品化, 建议 1-3 个月）
4. **GUI V1（Qt6, 最大缺口=VMware 感来源）**: src/windows/qtgui 骨架(108 行)→ 页签: 发行版/网络/设备/快照/镜像; 每页调 wsl.exe 命令面(零耦合契约); 目标"非技术用户开箱即用"
5. **运行时验证矩阵**: 快照/克隆/网络/USB/镜像每条命令在 VM 真机跑通(现仅冒烟)
6. **MSI 卸载验证**: SKIPMSIX 生效性(装-卸全链无报错)正式记录

## P2（增强, 后续）
7. OVN 控制面 v2（VXLAN 已 v0.1——ApplyTunnels; 控制面=文档 C9-OVN-DESIGN）
8. USB 深度(usbipd 全链细节/热插拔)
9. 自签证书+发布流程(PUBLISHING.md 已有流程脚本化)
10. Qt 面板接管 wsl-settings:// 协议(替换官方 Settings 入口——第二阶段)

## P3（远期/暂缓——决策记录, 别头铁重启）
11. 内核线(WSL2-Linux-Kernel 加驱动)——**搁置**(btrfs 模块够用)
12. QML 拓扑画布/设备管理器——等 GUI V1 站稳
13. "自己写 hypervisor"——**已论证不值**(见 ADR/讨论纪录: 引擎是现成的, 产品层才是战场)

---

## 已定决策记录（下家照此执行, 不要推翻）
- MSIX 根除(不引不产+SKIPMSIX=1+无内嵌)——硬决策
- 版本线 3.0.0.0, 规则=发布前 gh api 复核官方号, ≥我们则大版本+1——红线
- UpgradeCode 保持官方以覆盖升级——红线
- "替换式命令"→"叠加式"——(P0 正在改, 完工后本行作废)
- 上游合并机制: fetch→master ff→dev merge(零冲突保证=只新增独立文件+§9 修改点清单)——纪律
- 修改点盯梢: §9 清单(含 wix.in 2 处)——每次上游 merge 后检查
