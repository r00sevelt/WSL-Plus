# CLAUDE.md（项目级引导——人/AI 均适用）

## 这是什么
微软 WSL（MIT, 18.6 万行 C++）的 fork + 管理栈增强。**只做"加法"**：独立文件 + §9 修改点有限清单。
产物: GitHub Actions 出的单包 `wsl.3.0.0.0.x64.msi`（345MB, 覆盖式安装在官方 WSL 上）。

## 读仓库顺序（理解透彻的必要路径）
1. `docs/MANUAL-ONBOARD.md`（30 分钟上手）
2. `docs/ARCHITECTURE.md`（分层/端到端数据流——心智模型）
3. `docs/ADR-DECISIONS.md`（**先读决策档案再动代码**——防止推翻既有决策）
4. `docs/ROADMAP.md` + `docs/BACKLOG-DEFECT.md`（现状/缺口/排序）
5. `docs/WSLPLUS-DEVELOPMENT.md` §8-9（构建/版本红线/源码地图）+ `docs/GLOSSARY.md`（术语）
6. `docs/CHANGELOG-WSLPLUS.md`（22 轮演进+why）
7. 入口路径查 `docs/CALLGRAPHS.md`

## 硬纪律（违反=项目受损）
- **绝不 push origin**（微软上游）——只 push `plus`（r00sevelt/WSL-Plus）
- **上游合并只走** fetch origin → master `--ff-only` → dev `merge master`；冲突=停下 abort，绝不自动解决
- **零冲突机制** = 只新增独立文件 + §9「修改点(有限)」清单内的文件才能动——新增功能优先做新文件
- **MSIX 根除（不可逆）**：不产不引 MSIX/MSIXBUNDLE；wix 保持 SKIPMSIX=1 属性和无内嵌 Binary
- **版本红线**：PACKAGE_VERSION 必须 > 官方最新与自家已发布；发布前 `gh api repos/microsoft/WSL/releases` 复核
- **UpgradeCode 保持官方**（覆盖升级能力）——不要改成自己的 GUID
- **桌面命令兼容策略 = 叠加式**（官方 --xxx 全兼容 + 扩展子命令）——不要再改回"替换式"

## 工具链坑（AI 读代码前必知）
- 改 `build.yml` configure 行：**所有 `-DVAR=1.2.3.4` 必须双引号**（pwsh 拆 `.0.0.0`——22 轮教训）
- WiX 条件用**字面量小写 `true`**（不是 `ON`——19 轮教训）
- 消息发送: MessageWriter 模式=显式 `Send<T>(message.Span())`；结构体模式=`Send(message)`——不要混拼（16 轮教训）
- 快照相关类型：入参设置=`HostComputeEndpoint`、查询结果=`HNSEndpoint`、创建返回=`EphemeralHcnEndpoint`（字段`.Id`小写, 无 MacAddress）
- 云端 run 若排队被取消 → `gh workflow run -R r00sevelt/WSL-Plus "WSL-Plus Build Verify" --ref dev`（dispatch, 最稳）

## 验证（改完一处如何确认）
云端≈1h：push 后看 run；VM 里装 MSI → 全新发行版（旧盘是 ext4 不转 btrfs!）→ 快照链/网络/镜像/USB 各跑一条。
快照依赖 rootfs `/usr/bin/btrfs`（btrfs-progs）——缺就 apt 装（兼容性见 BACKLOG B6）。
