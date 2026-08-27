# WSL-Plus 演进记录（交接用：看这个比翻 git log 快）

> 每轮=「修复了什么 / 为什么」+「验证状态」。最新完整产物见云端 Actions Artifacts（单包 wsl.<版本>.x64.msi）。

| 轮次 | 内容（含根因） | 验证 |
|---|---|---|
| #1-#8 | 早期: Windows 侧 CLI 编译链打通（宽窄、PCH、FOLDERID、try/catch 等坑库 #1-#8，见手册 §8.2） | 编译闭环 |
| #9-#10 | 预检工具（臆造符号/裸 argv 拦截）+ 补 CATCH_RETURN 宏一致化 | 编码侧 |
| #11 | WslCoreVm 三根因: ModifyRequestType 用 hcs 域枚举 + SetCommTimeouts 传指针 + include WSLPlusNetworks.h | compile |
| #12 | 文档事实修正（公开仓/坑库补 #9-#12/C2664 归因/CMake 统一） | 文档 |
| #13 | WslCoreVm:1594 HNSEndpoint→HostComputeEndpoint（照 Bridged 模式, 返回值 .Id）; :2694 Send(message)→Send<Msg>(Span()); build.yml 增 upload | ✅ (13 轮 wslservice 编译过/链接) |
| #14 | MSI 全链: target=msipackage（依赖图全产品链+WiX）; 手册§8.5 打包红线 | 暴露 init 侧 5 错 |
| #15 | init 侧: main.cpp 补 include wslplus_snapshot.h + lambda 去未用捕获 Message | compile |
| #16 | Send 模式错位根治: 显式 Send<LX_MINI_INIT_SNAPSHOT_MESSAGE>(message.Span()) + 合并上游 00df65ad(零冲突) | 编译至 WiX 阶段 |
| #17 | WIX0103: wix 删 gluepackage.msix 内嵌 Binary（MSIX 根除落脚点）+ §9 登记 | ✅ 264MB 出包 |
| #18 | 开 WSL_BUILD_WSL_SETTINGS=ON（**未生效**——ON≠wix 字面 "true", 见 #19） | 264MB 不变 |
| #19 | WSL_BUILD_WSL_SETTINGS=true（对齐官方 build-job.yml:127）——WSL Settings 进包 | ✅ **346MB ≈ 官方 2.9.9.0(344.3)** |
| #20 | 安装/卸载致命 bug: 官方 MSIX 动作链缺 SKIPMSIX=1 → "无法删除 msix 程序包(错误:找不到包)"; 加 Property SKIPMSIX=1 整链跳过; 一并确认 wsldeps.dll 在包内(官方 NuGet WSL.Dependencies.amd64fre, wix:36) | 待云验 |
| #21 | 交付物定稿: 单包 wsl.3.0.0.0.x64.msi（configure -DPACKAGE_VERSION=3.0.0.0 + 官方式命名, 裸 exe 不再上传）; 文档全量化(§8.3 现状/§8.5 版本红线>官方+复核发布流程/§9 修改点 2 处/Settings 决策 §4.5); PUBLISHING 版本复核红线; 本 CHANGELOG | 待云验 |

## 版本线备忘
- 产品线: 3.0.0.0（> 官方 2.9.9.0; 规则=发布前 gh api 复核官方号, ≥我们→大版本+1）
- 上游跟踪: origin(微软)=master 已 25d3add3; 三次 0 冲突 merge（39551c56/00df65ad/25d3add3 共 200+ 文件）
- 交付: MSIX/MSIXBUNDLE 根除（构建线不引不产; SKIPMSIX=1 官方链跳过）
- GUI: 官方 WSL Settings=默认入口; WSL-Plus Qt 面板(独立)共存, 替换路线见 §4.5/组件 qtgui
