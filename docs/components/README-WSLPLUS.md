# WSL-Plus（README-WSLPLUS）

> 基于微软 WSL（MIT）重构增强: 快照/克隆、企业级网络、外设共享、镜像库、图形管理。
> 底层=Windows Subsystem Linux；Plus=增强分署；Max=远期完全体。只产 exe+msi（MSIX 根除）。

## 快速开始
```
1. 构建环境: VS 2026(MSVC v180 + ATL + Clang组件22.1.3 + Spectre库) + CMake ≥3.25（推荐 4.4.x, 同手册 §8.1）
   （详见开发手册 §8.1 —— 含每个组件的确切选择）
2. configure: cmake . -A x64 -DWSL_SKIP_GLUE_PACKAGE=ON -DWSL_DISABLE_PCH=ON -DCMAKE_BUILD_TYPE=Release
3. build: cmake --build . --config Release --target wsl wslservice -- /m:4
4. 安装: msiexec /i wsl.msi（覆盖升级官方版,数据保留）
```

## 文档地图（全部文档入口）
```
总纲: 开发手册.md（16章,架构/API/坑库/测试矩阵/交接表）—— 从这里开始
组件（本目录）: 每组件独立文档（API/依赖/扩展点）
流程: 开发准则(五原则+两条红线) | PUBLISHING(发布升级回滚) | NETWORK-PHASE-DESIGN
     | USBDEV-STUDY | C9-OVN-DESIGN | 任务清单(40+11粒度) | 审计日志
```

## 目录结构（我们新增件）
```
src/windows/common/WSLPlus*.h/.cpp   5 个模块(见组件文档)
src/windows/qtgui/*                   Qt GUI 骨架(独立)
src/linux/init/wslplus_snapshot.*     guest 快照模块
docs/*                                本套文档
```

## 分支与云纪律
```
master=微软上游(干净) · dev=我们全部 · feature/*=开发(零云)
云端动作(推送/合并/取消/检查)=一律用户指令（避免预算浪费——见开发准则）
```
