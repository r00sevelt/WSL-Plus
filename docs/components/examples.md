# 场景示例大全（examples）

> 全部命令可直接复制（PowerShell; 单引号规范）。

## E1 快照保护（改系统前的保险）
```powershell
wsl snapshot create web preleap
wsl exec web -- apt upgrade        # 放心改
wsl snapshot restore web preleap   # 翻车了? 重启即回
```

## E2 克隆开发环境（模板复用）
```powershell
wsl clone base dev1
wsl clone --full base prod1        # 独立副本
wsl clone dev1 snaptest --基于快照? (v0.2—先 snapshot 再 clone 链)
```

## E3 企业网络模板（三网拓扑）
```powershell
wsl network add corp --type bridge --cidr 192.168.0.0/24
wsl network add mgmt --type nat --cidr 172.16.0.0/24 --dns 172.16.0.1
wsl network attach web corp; wsl network attach web mgmt
wsl network ports-on web
# guest /etc/wslplus-static.conf: "eth1 192.168.0.10/24 192.168.0.1"(桥接静态)
```

## E4 VLAN 隔离（部门）
```powershell
wsl network add vlan-sales --type bridge --vlan 100
wsl network attach app vlan-sales
# guest /etc/wslplus-vlan.conf: "eth1 100"
```

## E5 串口接固件/设备
```powershell
wsl device add com1 serial COM3
wsl device attach com1 app    # guest /dev/ttyS1
wsl device eject com1         # 归还 Windows（弹出式）
```

## E6 镜像维护/迁移
```powershell
wsl image import ubuntu-mini.tar ubuntu-base --desc 开发基线
wsl image list
wsl image rm ubuntu-base
```

## E7 ACL/QoS 上线保护
```
# guest /etc/wslplus-acl.conf:  "drop tcp 0.0.0.0/0 22" 只允许来源
# guest /etc/wslplus-qos.conf:  "eth1 100"  下载限 100M
```

## E8 多机互联（VXLAN）
```
# A机: /etc/wslplus-tunnel.conf "eth1 100 10.9.9.1"
# B机: "eth1 100 10.9.9.2" → ip link 见 vx100; ip addr/静态 → 互通
```

## 通用：devices watch + 规则（自动共享）
```powershell
wsl device add usbstick usb usb:xxx --policy auto --instance web
wsl device watch   # 插入 U 盘 → 自动 attach；要归还 eject
```
