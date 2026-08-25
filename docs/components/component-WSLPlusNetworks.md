# 组件: WSLPlusNetworks（网络配置模型）

## 概述
企业级网络拓扑的**配置面**：networks.yaml 的 CRUD/校验 + 实例-网络绑定。数据模型被 CLI、GUI、服务端（ApplyExtraNetworkAdapters/ApplyPortMappings）共同消费。

## 位置
`src/windows/common/WSLPlusNetworks.{h,cpp}` | 配置 `.wslplus/networks.yaml`

## 数据结构
```cpp
NetworkPortMapping { hostIp, hostPort, guestPort }
NetworkConfig { name, type∈{nat,bridge,host-only}, cidr, dns, vlan?, ports[] }
```

## 公共 API
| 签名 | 语义 |
|---|---|
| `ConfigPath()` | ~\.wslplus\networks.yaml |
| `Load()` | 全量（无文件=空；解析失败 throw） |
| `Save(cfg)` | 同 name 覆盖（先 Validate） |
| `Remove(name)` | 未找到 throw E_INVALIDARG |
| `Attachments(instance)` / `Attach(instance, net)` / `Detach(instance, net)` | 实例-网络绑定（幂等） |
| `Validate(cfg)` | name 非空/type 三值/端口 1-65535 |

## 依赖
yaml-cpp（common 共享；CMake: set_source_files_properties + target_link common）

## 与执行层的边界（分离原则）
本模块只做"配置档案"；**HNS 端点/端口转发的执行在 WslCoreVm**（见服务端扩展组件）。GUI/CLI 调用本模块做编辑 → 服务端消费其产物。

## 扩展点
新网络类型/字段=刷新结构+Validate+Schema（如加 OVS 类型——预留 v2）

## 测试归属
T3/T4/T8（配置→生效链路）
