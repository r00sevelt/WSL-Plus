# WSL-Plus 交接手册（30 分钟上手版）

> 目标: 让下一位开发者从零到"能改代码→能出包验证"只用 30 分钟。文档齐全度: 手册(16 章)+组件(8)+流程(5)+本册+ROADMAP+BACKLOG。

## 1. 这是个什么项目（10 秒理解）
- 基底=**微软官方 WSL 源码**（MIT, 18.6 万行 C++），我们**只添加不改写**（零冲突 merge 纪律）
- 加的是: **管理栈**——快照/克隆/企业网络/USB/镜像/未来 GUI（VMware 级体验, 跑在 WSL2/Hyper-V 客户栈上）
- 产出: GitHub Actions 出的**单个 `wsl.3.0.0.0.x64.msi`**（345MB, 覆盖式安装到官方 WSL 上）

## 2. 环境（一次性）
- Windows 11 + VS 2026 Enterprise（组件: MSVC v180/ATL/Clang 22.1.3/x64+x86 Spectre 库——手册 §8.1）+ CMake ≥3.25
- git 双 remote: **origin=微软上游（fetch 用） / plus=我们（push 用）**——千万别 push origin

## 3. 本地 vs 云（重要）
- **本地不编译整套**（18.6 万行+1 小时+内存要求高）——写代码产物全走**云端 Actions**
- 云端一轮≈1h（msipackage 全链+WiX 打包）
- **push 触发易被排队取消**（20 轮教训）→ 重跑用:
  ```powershell
  gh workflow run -R r00sevelt/WSL-Plus "WSL-Plus Build Verify" --ref dev
  ```

## 4. 改代码的边界（零冲突纪律=项目生命线）
- **加**: 新文件（WSLPlus*.h/.cpp, docs/*, src/linux/init/wslplus_snapshot.*, qtgui）→ 永不冲突
- **可改(盯梢点)**: §9「修改点(有限)」清单（CMakeLists 开关、WslCoreVm 4 扩展、main.cpp 网络/快照段、wix.in 2 处……）——**只有这些文件动上游本体**
- 上游合并: `git fetch origin && git merge --ff-only origin/master`(master) → `git checkout dev && git merge master`——冲突=停下 abort 报告, 绝不自动解决

## 5. 学这堆代码的入口顺序（1 小时内）
1. `docs/CHANGELOG-WSLPLUS.md`（11-22 轮干了啥, 为什么）
2. `docs/WSLPLUS-DEVELOPMENT.md` §8(构建/CI/版本)+§9(源码地图)
3. `docs/ROADMAP.md`（下一步干哪件）
4. 代码: `src/windows/common/WSLPlusCommands.cpp`(入口) → `WSLPlusNetworks/Usb/Images.cpp`(业务) → `src/linux/init/wslplus_snapshot.*`(guest 侧) → `WslCoreVm.cpp` 的 ApplyExtraNetworkAdapters/SnapshotDistribution

## 6. 单测路径（改完了一处怎么验）
VM(WSL2 需嵌套虚拟化) 装 MSI → 全新发行版(只有新盘才是 btrfs!) → `wsl snapshot create test1` / `list` / `restore` / `delete` → 网络/镜像/USB 各跑一条
- 已知坑: ①老发行版是 ext4 不转 btrfs ②快照命令依赖 rootfs 里 /usr/bin/btrfs(btrfs-progs) ③官方 .wsl/tar 导入=自动 btrfs 布局, 不用自建 rootfs

## 7. 22 轮经典坑速查（别重踩）
| 坑 | 预防 |
|---|---|
| ON≠"true"(WiX 字符串比较) | 传参用小写 `true` |
| pwsh 拆 `.0.0.0` | `-DVAR=1.2.3.4` 加双引号 |
| Send(message) vs Send<T>(Span()) | MessageWriter 模式显式模板参数 |
| HNSEndpoint/HostComputeEndpoint | 创建设置=HostComputeEndpoint, 查询结果=HNSEndpoint, 创建返回=Ephemeral(无 MacAddress) |
| MSIX 动作链找不到包 | SKIPMSIX=1 已设(别删!) |
