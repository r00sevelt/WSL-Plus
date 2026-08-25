# 调试与排障卷（debugging）

## 1. 日志来源
| 源 | 取法 |
|---|---|
| WSL 服务端 | `wsl --debug`（微软插件状态）/ 事件查看器: Microsoft-Windows-WSL/Lxss 运行日志 |
| guest dmesg | `wsl -d <distro> -- dmesg`（kernel/btrfs/网络引擎） |
| 我们的模块 | 服务端 WSL_LOG（etw）/ 客户端打印（GetSystemErrorString） |
| 网络引擎 | WslCoreVm 日志（TraceLogging）→ Windows 性能记录器（wpr + WSL 配置） |
| usbipd | `usbipd list` 直查（Available 探测失败时先查此） |

## 2. 运行时排障速查（逐步）
| 症状 | 步骤 |
|---|---|
| 快照无效果 | ①guest `btrfs subvolume list /` 看 @snap-；②`which btrfs`（缺→E5 静态注入）③服务端日志 MiniInit Snapshot 码 |
| 克隆后异常 | ①注册表新项存在？②CLONE flag→guest `ls /etc/wslplus`；③差异盘父链路 |
| 网卡不出现 | ①服务端日志"extra network adapter"；②guest `ip link`(等 10s)；③vSwitch 名拼写 |
| 端口转发不通 | ①`netstat -ano | findstr <hostPort>` 有监听；②guest 内服务属实；③bindingIp |
| 串口不回显 | ①COM 占用（device manager）②`guest ls /dev/ttyS1` ③桥线程日志 |
| 设备 eject 未归还 | ①桥线程仍挂（停进程/kill）②Modify Remove 是否成功（服务日志）③registry devices.yaml 清空？ |

## 3. 性能调优
- 快照耗时= btrfs 子卷（毫秒级；若慢→磁盘类型/同步）
- 网络吞吐= usbipd/桥接是瓶颈显示；对比直连差异
- 内存（编译期）= PCH 禁用已控；/m 并行=云上 4

## 4. WinDbg 接入（内核调试附加）
- 测试 VM 装 Debugging Tools（WDK）→ kdnet 配置 → 服务崩溃时连内核（我们服务=用户态为主, 常规 EventLog 优先）
