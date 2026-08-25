# WSL 原生机制适配（wsl-native）

> 说明: 我们未改以下机制（只列"用户可用/与我们交互"的内容）——完整细节见微软 WSL 官方 docs（wsl.dev）。

## 1. WSLg（GUI 通道）
- 机制: virtio shm + Wayland/Pulse（WSLg 组件）→ Windows 桌面投射
- 我们: 未改动；`wsl launch` 默认带 GUI 支持（EnableGuiApps 传给 VM 配置）
- 我们的 guest 若跑桌面: 系统需含 wslg-init（发行版通常自带）

## 2. GPU/CUDA（共享分配）
- Windows 侧 vGPU → guest 驱动的 Dev.libs; 我们未动——共享 GPU 场景开箱可用

## 3. 互操作
- 双方向命令互调用（wsl.exe <-> guest binaries）、WSLENV、/mnt/drvfs
- 我们的 CLI 作为 wsl.exe 子命令自然参与；互操作不受影响

## 4. 挂载与共享
- drvfs（/mnt/c）、virtiofs（共享目录）、plan9（我们 init 含 plan9 服务器源码——未改）
- 要共享目录: 用官方机制（.wslconfig [automount/挂载配置]）

## 5. localhost / 端口
- localhost 转发=官方（wslrelay）; 我们的 ports-on 增加"宿主端口→转发"（额外层）

## 6. 与我们的文档边界
- 官方机制的深入技术细节 → 查询 wsl.dev（wsl.exe/wslservice/wslg/relay/plan9 页面）；
- 我们的差异点(任何 WSL+ 改动) → 见本套文档（组件/手册）

## 7. 已知不适用
- Windows 容器（Hyper-V 隔离）→ 不在范围；Windows guest → 架构不支持（见手册 §11.5）
