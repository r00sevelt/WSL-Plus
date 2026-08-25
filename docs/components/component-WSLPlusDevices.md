# 组件: WSLPlusDevices（设备管理模型）

## 概述
外设接入的**配置面**：设备目录 + 实例绑定 + **三档策略**（shared-first 设计: auto 自动共享/manual 手动/dedicated 独占直通）——设备生命周期语义 attach(挂载)/eject(归还,类似弹出 U 盘)/rm(仅清登记,绝不碰硬件)。

## 位置
`src/windows/common/WSLPlusDevices.{h,cpp}` | 配置 `.wslplus/devices.yaml`

## 数据结构
```cpp
enum class Policy { Manual, Auto, Dedicated };
DeviceConfig { id, type∈{usb,serial,parallel}, hostPath, instance, policy }
```

## 公共 API
| 签名 | 语义 |
|---|---|
| `Load()/Save(cfg)/Remove(id)/Validate(cfg)` | CRUD（同网络模块模式） |
| `SetPolicy(id, policy, instance)` | 改绑/策略（auto 时供 WatchLoop 消费） |

## 与通道层边界
本模块=登记；**通道执行**(串口 HCS 桥/USB 后端 attach-eject)在 WslCoreVm/WSLPlusUsb——策略层决定"何时调用后端"。

## 安全语义（本组件灵魂）
```
eject = 归还 Windows（优雅释放→松绑→manual 空实例）
rm   = 仅删除登记记录（硬件/驱动/Windows 侧零触碰）
```

## 测试归属
T5/T6 + watch 场景（插入→auto 策略→自动挂载→eject 归还）
