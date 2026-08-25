# 维护与运营（maintenance）

## 1. 上游例行（双轨）
```
周期: 每周/上游有更新时
① git fetch origin → 检查 master..origin/master
② checkout master && merge --ff-only origin/master
③ checkout dev && merge master（冲突=极少; 按 ADR#8 语义）
④ （经用户指令）push dev
```

## 2. 文档维护流程
- 每次功能/坑/API 变化 → 三处同步: 开发手册（对应章）+ 组件文档 + 任务清单状态
- 手册版本递增（v0.x changelog）；组件文档签名取自头文件（API 变化即更新）
- 排雷结论 → 手册 §8.2（新坑加行）

## 3. 版本与依赖
- 构建依赖清单（见手册 §8.1）——VS 组件变更时更新
- NuGet 依赖: 微软 WSL Dependencies（configure 自动恢复）；FetchContent 包 yaml-cpp/gsl——需更新时改 CMakeLists FetchContent 版本
- 无 vcpkg（除非 libusb 后端 v2——届时登记）

## 4. 发布节奏（参考）
- 每批功能绿后打 tag（tag=构建版本: v3.0.0 等——版本红线 §8.5）
- 发布物: wsl.msi + exe 族（actions 下载）→ PUBLISHING.md 步骤

## 5. 安全巡检
- 依赖漏洞（yaml-cpp/gsl 更新追踪）
- usbipd 外部组件=其上游安全
- AMA: 权限检查（无降权场景——本产品用户态）

## 6. 运营指标（可选）
- CI 时长/预算（GitHub 2000×2/月；建议挂 Azure 备胎见手册 §8.3）
- 问题数/修复周期（审计日志统计）
