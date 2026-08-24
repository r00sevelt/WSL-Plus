# WSL-Plus 网络执行层阶段设计（C3b–C8）

> 2026-08-24 · 目标: vSwitch/企业网络拓扑（ESXi/Hyper-V/PVE 级）执行面一次成型
> 前置已完成: C1 数据模型 / C2 编辑 CLI / C4a 实例绑定（配置面 ✅）

## 1. 阶段结构（依赖序）

```
共享基础设施（本阶段先做）:
  I1 端口转发服务（宿主监听 + wslrelay 会话）
  I2 HCN 端点/网络生成器（配置 → HNS/HCN 运行时）

执行块（每块=独立提交=独立验证）:
  C3b NAT 端口映射落地   → I1 + HNS proxy    （端口直达工作）
  C4b1 Windows 多 vNIC   → I2 + HCS 端点数组 （guest 可见多网卡）
  C5  桥接增强           → BridgedNetworking 底座增强（同网段 DHCP/静态）
  C6  VLAN               → HCN 端口 TAG（access/trunk）
  C7  ACL                → HCN ACL（5元组规则）
  C8  QoS                → HCN 带宽参数
  C4b2 guest 多接口配置（Linux 侧第二网卡 DHCP/静态，阶段末）
```

## 2. 共享基础设施设计

### I1 端口转发服务
```
复用先例:
  WSLCPortMapping（HostPort/ContainerPort/Family/Protocol/BindingAddress）
  LaunchPortRelay（helpers.cpp:638 → wslrelay.exe PortRelay 会话）
实现: WSLPlusNetworkRuntime（common）
  - 运行状态: 每实例已启动转发链（host:port → vm:guestPort）
  - 启停接口: Apply(instance, ports) / Clear(instance)
执行: 宿主监听 socket（bind hostPort）→ accept → LaunchPortRelay(socket, VmId, userToken)
```

### I2 HCN 生成器
```
HCN 先例: NatNetworking::CreateNetwork / BridgedNetworking
映射: NetworkConfig{type,cidr,dns,ports} → HCN 网络/端点 JSON
  type=nat     → HcnCreateNetwork(nat)（类比 CreateNetwork）
  type=bridge  → bridged 端点（复用 BridgedNetworking）
  type=host-only → internal 网络（HNS internal switch）
```

## 3. 执行块接线要点

| 块 | 接线 | 改动面 |
|---|---|---|
| C3b | I1 配置 → Apply（ports→宿主端口监听+relay） | WSLPlusNetworkRuntime + svccomm(ApplyPortMappings) + CLI `wsl network ports-on/off <instance>` |
| C4b1 | 附件网络 → 额外 HCN endpoint + HCS 阵列（WslCoreVm 网络配置段） | WslCoreVm + I2 |
| C5 | BridgedNetworking 增强（dhcp 客户端/手动 IP 设置） | BridgedNetworking.cpp（开源底座） |
| C6 | 端点 TAG 参数（Access=port VLAN id） | I2 的 endpoint JSON + guest 侧 VLAN 配置 |
| C7 | HCN ACL 规则（per network） | I2 + svccomm(SetNetworkAcl) |
| C8 | HCN 带宽（per NIC） | I2 + svccomm |
| C4b2 | guest 多 NIC（Linux init 网络段“第二接口 DHCP/静态+路由”） | src/linux/init（config/network 段） |

## 4. 提交切割与验证

```
每块一提交（≤5 文件），验证点:
  C3b: 配置 2 端口 → 宿主访问转发到 guest ✅
  C4b1: HCS 配置含 2 端点（日志/CI 编译验）
  C5: guest 桥接网卡拿同网段 IP（VM 测试）
  C6/C7/C8: 配置→HCN JSON 生效（VM 验证）
由 CI 编译轮兜底 + VM 行为验收（E7）最后跑
```

## 5. 风险与红线（既定）
- Wi-Fi 同网段桥接: 平台红线（不承诺）
- C4b2（guest 第二接口）: 微软 init 网络段单接口为设计；第二接口需自己扩展——排在阶段末（VLAN/C4b2 共用此改动）
- OVN/C9: 远期（utility VM 内 OVN + 宿主 bridged 对接）
