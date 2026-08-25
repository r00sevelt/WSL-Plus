# 组件: WSLPlusUsb（USB 后端抽象 + 守护）

## 概述
"多协议后端"的 USB 接入（QEMU 哲学）：接口抽象 + v1 usbipd-win 实现 + 热插拔守护（商业"无缝感"）。

## 位置
`src/windows/common/WSLPlusUsb.{h,cpp}`

## 公共 API
```cpp
enum class BackendKind { Usbipd, Usbredir /*预留*/ };
class IUsbBackend {
  virtual BackendKind Kind() const noexcept = 0;
  virtual std::vector<std::string> Enumerate() const = 0;
  virtual void Attach(const std::string& busId, const GUID& vmId) = 0;
  virtual void Detach(const std::string& busId) = 0;
  virtual bool Available() const noexcept = 0;
};
std::unique_ptr<IUsbBackend> CreateDefault();   // usbipd 进程封装
std::unique_ptr<IUsbBackend> CreateUsbredir();  // 接口壳(E_NOTIMPL)——见 USBDEV-STUDY
void WatchLoop(backend, intervalSeconds, onNewDevice);  // 轮询 diff 回调
```

## 实现要点
- UsbipdBackend: `Available` 查 usbipd.exe(System32)；`Enumerate` 解析 `usbipd list`（取 busId 行）；`Attach/Detach` 组装命令（busId/vmGuid），失败 WSL_LOG+E_FAIL
- WatchLoop: 5s 快照 diff → 新设备回调(策略层决定 attach); 持久守护进程用

## 边界
- usbipd-win=外部组件（安装属用户许可项）；可用性由 Available 探测
- usbredir=接口壳（Win 侧无官方 server——详见 docs/USBDEV-STUDY.md；v2 评估 lusb/WinUSB 后端）

## 测试归属
T6（需 usbipd 环境）; watch 场景=脱 UI 冒烟
