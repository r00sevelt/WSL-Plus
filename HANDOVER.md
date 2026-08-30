# WSL-Plus 交接说明（前任遗留 / Handover from previous owner）

> 本仓库已由前任（项目发起人）完整交接。**以下三步是前任留下的交接工作清单**——按顺序执行即可无缝接手。

## 交接三步

1. **克隆并切换主线分支**
   ```powershell
   git clone https://github.com/r00sevelt/WSL-Plus.git
   cd WSL-Plus
   git checkout dev        # dev = 全部代码+文档的主线; master = 官方上游镜像(只读跟踪)
   ```

2. **读两份入口文档（30 分钟）**
   - `CLAUDE.md` —— 读序 7 步 + 硬纪律（绝不 push origin / 零冲突机制 / MSIX 根除不可逆 / 版本红线 / 工具链坑）
   - `docs/MANUAL-ONBOARD.md` —— 30 分钟上手（环境/双 remote/云构建流程/边界纪律/经典坑速查）

3. **按路线图开工**
   ```powershell
   docs/ROADMAP.md         # 工作排序: P0(P1 命令叠加+帮助总表+品牌) → P1(GUI V1 Qt6+验证矩阵) → V1 竣工验收单
   docs/BACKLOG-DEFECT.md  # "还差什么"的准确账本(功能/验证/工程缺口, 每项带验证方式)
   docs/DIFF-VS-UPSTREAM.md# 与官方 2.9.9.0 的差异总表(上游合并后必须核对此表)
   ```

## 前任已交付（2026-08 状态）
- **代码**: 22+1 轮闭环——快照/克隆/企业网络栈/USB/镜像/btrfs 根盘(引擎级全通)
- **产物**: `wsl.3.0.0.0.x64.msi`（345MB 单包, GitHub Actions Artifacts 可下载, 覆盖式安装官方 WSL）
- **文档**: 30+ 份(手册/组件/流程/交接套件/深度篇/决策档案 ADR#1-12/术语/调用链) + 本文件
- **工程纪律**: 上游合并 4 次零冲突机制; 决策与坑已全部文档化——**请勿推倒重来**(先读 ADR-DECISIONS)

## 前任未交付（=你的工作项）
- 产品化层全部: P0 三件(命令叠加式/帮助总表/品牌) + GUI V1(最大缺口) + 验证矩阵 B1-B6 全绿
- 详细清单: `docs/ROADMAP.md` P0-P3（附竣工验收单）

## 联系方式
- 上游仓库: https://github.com/microsoft/WSL (merge 用, 勿 push)
- 本仓库 issue: https://github.com/r00sevelt/WSL-Plus/issues (交接问题欢迎提出)

—— 前任 2026-08-31
