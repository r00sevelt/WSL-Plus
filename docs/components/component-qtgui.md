# 组件: qtgui（WSL-Plus Desktop，Qt 骨架）

## 概述
图形管理前端（N0-GUI v0.1）：独立 Qt6 Widgets 子工程（不依赖 common 大库——**通过 wsl.exe 命令面对话**，零耦合）；后续扩展 QML 拓扑画布/设备管理器。

## 位置
`src/windows/qtgui/{CMakeLists.txt, main.cpp}`

## 构建（独立）
```
cmake -S src/windows/qtgui -B build-gui -DCMAKE_PREFIX_PATH=<Qt6安装路径>
cmake --build build-gui --config Release
```
产物: wslplus-desktop.exe | 依赖: Qt6 Widgets（构建机需装 Qt6；CI 可选步骤装）

## 当前页签
| 页签 | 功能 |
|---|---|
| 发行版 | 列表 + 刷新 + 打开终端（调用 wsl.exe） |
| 网络 | wsl network ls 展示（调命令面） |
| 设备 | wsl device list 展示 |

## 扩展路（v0.2）
- QML Canvas 拓扑画布（消费 networks.yaml → nodes/edges 渲染 VMware 式编辑器）
- 直接链接模块（将来 WSLPlus 模块 DLL 化）替换命令面

## 测试
T10（骨架冒烟: 三个页签触发命令返回非空）
