# 测试运行指南（testing）

> 测试矩阵定义见开发手册 §10（T1-T9+T10）。本篇=怎么实际跑。

## 1. 环境准备（一次性）
```
- 宿主机: 安装本 MSI（wslservice 替换）+ 一个 Ubuntu 发行版
- guest: apt install btrfs-progs（确保 mkfs.btrfs；静态注入 E5 后免）
- 网络: 预置 vSwitch（可 DNS 命名 bridge）；usbipd（仅 T6）
- 测试机: 我们仓库 feature 分支的 wsl.exe（CI 产物——本地不构建, 编译全在云端 Actions）
```

## 2. T1（快照全链脚本化）
```powershell
wsl snapshot create test1
wsl snapshot create test1 guarded
wsl exec test1 -- bash -c 'echo Broken > /etc/break.txt'
wsl snapshot restore test1 guarded     # 保底 prestore + 默认子卷
wsl shutdown test1; wsl start test1
wsl exec test1 -- ls /etc/break.txt   # 期望: 不存在（回到快照）
wsl snapshot delete test1 guarded
```

## 3. T2（克隆）
```powershell
wsl clone test1 clonetest; wsl start clonetest
wsl exec clonetest -- hostnamectl   # 期望: 与源不同（唯一化）
# 删除 clonetest: wsl unregister clonetest（旧命令保底可用）+ 差异盘链路验证
```

## 4. T3-T4（网络/端口）
```
# 预置: networks.yaml 每项 + attach 实例 → wsl network ports-on
# 验证: host curl 8080 → guest 服务; guest ip link（eth0+eth1）
# VLAN: /etc/wslplus-vlan.conf "eth1 100" → ip link 见 eth1.100 + dhcpcd 拿租约
```

## 5. T5-T7（设备/镜像）
```
# 串口: 连接真实 COM（或虚拟 COM 对）→ guest ttyS1 echo 回环 → eject 归还
# USB: usbipd bind 一个 U 盘 → watch（并发）插入即挂；eject 归还
# 镜像: wsl image import <官方分发tar> myimg → list；（将来 launch --image 用例）
```

## 6. 验收环境（E7 真机计划）
- 虚拟机：VMware 实例（嵌套）或双机——先跑 T1-T4（本套文档即验收脚本）

## 7. CI 触发
- push dev → build.yml（wsl+wslservice /m:4）；结果=产物/错误流；**云端纪律=用户指令**
