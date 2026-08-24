# C9: OVN 化路线设计（v0.1 数据面已落地）

> 2026-08-25 · v0.1 = VXLAN 隧道（Linux 原生，已在 init 实现）；
> 本文件为"OVN 化"的完整路线（v2 控制面）

## v1 已交付（VXLAN 数据面）
```
/etc/wslplus-tunnel.conf: <phyIface> <vni> <remoteIp>
→ ip link add <iface>.vx<VNI> type vxlan id VNI remote <remoteIp> dstport 4789
（与 OVN/Ovs 的 VXLAN 隧道同构 —— 这就是分布式网络的底层隧道原语）
```

## v2 控制面（ovn 化）—— 分块
```
① 控制面（utility VM 内）:
   ovn-northd(逻辑网络 DB) + ovn-controller(每 host)
   → 用 WSL utility VM 作 OVN 控制面主机（我们已有 VM 架构,零新增硬件）
   逻辑网络声明（logical switch/router/port）落成 OVSDB
② 数据面: 我们的 VXLAN 隧道接口 + 逻辑 switch 绑定（ovn-controller 生成流表）
③ Windows 侧桥接: utility VM ↔ 宿主网段（VM 外网段互连）→ 分布式网络出口
```

## 为什么分两步（科学理性）
```
VXLAN = 数据面原语（隧道/隔离/逻辑分段）—— 一次做完就够 80% 用户价值
OVN 控制面 = 逻辑编排（声明式拓扑/自动流量转向）—— 重活，价值集中在
  多主机场景; 单机/少机用户 v1 隧道+多网卡+VLAN 已覆盖其效果
→ v1 把"能力"做掉; v2 按需求（多主机时）加控制面
```

## 验收
- v1: guest 内 vx 接口 up + 对端 ping（两台 WSL-Plus 隧道互通）
- v2: logical switch 声明→自动建隧道+流表（多宿主场景）
