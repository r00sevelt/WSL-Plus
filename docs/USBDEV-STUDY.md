# USB 后端生态研究（usbredir / libusb 路线）

> 2026-08-25 · 回答"usbredir 后端能不能真实现"的现实核查

## 结论
```
usbipd-win（已实现 v1）: USB/IP 标准协议 + 微软官方维护 = 当前最优
usbredir（预留）: QEMU/SPICE 生态协议; 但 WIN 宿主没有官方 usbredirserver
                  —— 故 v2 真实现的两条路径:
  路径 A: 引入 usbredirserver-win 运行时（第三方移植, 稳定性待验证）
  路径 B（推荐）: 实现 "WinUSB/Libusb 后端"（用户态直通,
               = QEMU Linux usb-host 的 Windows 对应; libusb + WinUSB filter)
```

## 各方案对照
| 后端 | 协议 | Win 侧现状 | 我们的状态 |
|---|---|---|---|
| usbipd（USB/IP） | 网络化共享 | 微软官方 | ✅ v1 已实现 |
| usbredir（SPICE） | 网络化共享 | 无官方运行时 | 🔜 保留接口/待运行时 |
| libusb/WinUSB | 用户态直通 | 成熟（libusb 支持 Windows） | 🔜 v2 推荐路径 |

## v2 计划（记录）
- [ ] 评估 libusb（vcpkg 接入 common 依赖）→ 实现 LibusbBackend（枚举/attach via WinUSB)
- [ ] 若市场有新 usbredirserver-win → 优先评估
- [ ] 三后端共接口（已有: IUsbBackend）
