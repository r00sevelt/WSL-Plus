# 组件: WSLPlusCommands（CLI 分发层）

## 概述
「替换式命令集」的调度中枢：解析 `wsl.exe <子命令>` 参数 → 分发到各业务模块；同时是**宽窄边界层**（argv=宽、模块接口=窄——转换集中于此）。

## 位置
`src/windows/common/WSLPlusCommands.{h,cpp}`（common 库，链进 wsl.exe）

## 职责
| 子命令 | 委托目标 |
|---|---|
| snapshot create/list/restore/delete | SvcComm::SnapshotDistribution（→ guest btrfs） |
| clone [--full] | SvcComm::CloneDistribution |
| network ls/show/add/rm/attach/detach/ports-on | networks 模块 + SvcComm::ApplyPortMappings |
| device list/add/rm/auto/attach/eject/watch | devices 模块 + SvcComm(serial/backends) + WatchLoop |
| image list/import/rm | images 模块 |

## 公共 API
```
std::optional<int> Dispatch(_In_ const std::wstring& commandLine);  // 未命中=nullopt(走微软旧逻辑)
```
入口: `WslClient::WslMain`（MSIX 检查后插入 Dispatch 调用——见 WslClient.cpp）

## 边界助手（本组件核心设计）
```
std::string Narrow(LPCWSTR)  → 宽→窄(模块接口)
std::wstring Widen(const std::string&) → 窄→宽(服务宽接口)
```
**规则: 模块声明用窄(std::string)=YAML/Linux 原生；服务接口用宽(LPCWSTR)；转换只在 CLI 层**

## 退出码/错误
- 0 成功 / -1 用法或模块 throw（错误经 GetSystemErrorString 打印）

## 扩展点
新子命令 = 加一个 verb 分支（image/device 已为范本）+ 业务调用；遵循"≤5 文件/≤300 行子拆"红线

## 测试
T1-T9（测试矩阵 §10）+ 冒烟: 双 shell(PS/cmd) 各子命令空参出用法
