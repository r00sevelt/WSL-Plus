# 组件: 服务端扩展（WslCoreVm / ILxssUserSession / SvcComm / idl）

## 概述
WSL-Plus 在微软服务栈上的 5 个扩展接口（各层一一对应）——全部遵循**原版 IFACEMETHOD 范式**（m_session.lock + ServiceExecutionContext；禁止臆造辅助函数）。

## 接口层（idl → svccomm → session）
| 方法 | 语义 |
|---|---|
| `SnapshotDistribution(Guid, HANDLE output, char* action, char* name)` | 快照命令入 guest（output=list 文本管道） |
| `CloneDistribution(Guid, LPWSTR newName, BOOLEAN full)` | 克隆（链接/完整+注册复制+CLONE 标志） |
| `ApplyPortMappings(Guid, const char* portsJson)` | 端口映射应用（监听+relay） |
| `AttachSerialPort(Guid, LPWSTR com)` / `DetachSerialPort(Guid)` | 串口重定向挂载/归还 |

## 执行实现（WslCoreVm——微软服务类的扩展方法）
| 方法 | 要点 |
|---|---|
| `SnapshotDistribution(lun, output, action, name)` | MiniInit 消息+双 Accept 通道（list 输出） |
| `AttachSerialPort(com)` | CreateNamedPipe→Modify(Add ComPort)→桥线程(COM↔pipe 115200); eject 停桥+Remove |
| `DetachSerialPort()` | 优雅释放→Modify(Remove)→归还 |
| `ApplyExtraNetworkAdapters()` | attachments bridge 网络→HCN 端点→Modify(Add NetworkAdapter)(多vNIC) |

## 其他配套改动
- `WslCoreFilesystem::CreateLinkedVhd`（VERSION_3 差异盘=链接克隆核心）
- 消息协议: lxinitshared.h 增 LxMiniInitMessageSnapshot(+Response) 与 0x40 flag
- 分发标志: LXSS_DISTRO_FLAGS_WSLPLUS_CLONE(0x10)→miniInit 0x40→guest 唯一化

## 关键约定
- 路由=注册项→configuration→`_CreateVm()`→AttachDisk(LUN)→扩展方法→Eject（照 ResizeDistribution 范式）
- 错误: THROW_HR/WSL_E_* + CATCH_RETURN 写入出参 Error

## 测试归属
T1/T2/T3/T5（接口→链路）
