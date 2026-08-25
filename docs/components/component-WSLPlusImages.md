# 组件: WSLPlusImages（本地镜像库）

## 概述
rootfs 镜像的私有存档/分发——"实例快照/根卷"的库（incus image 定位）：导入官方/自有 tar → manifest 入库 → list/rm；未来 launch 自镜像实例化。

## 位置
`src/windows/common/WSLPlusImages.{h,cpp}` | 库 `~\.wslplus\images\<name>\{rootfs.tar.gz, manifest.yaml}`

## 数据结构
```cpp
ImageManifest { name, description, architecture="x64", baseVersion, createdAtUtc }
```

## 公共 API
| 签名 | 语义 |
|---|---|
| `ImagesRoot()` | 库根路径（用户主目录——userenv API, 见 §8.2 坑#7 绕法） |
| `List()/Read(name)` | 扫描(跳过无 manifest 目录)/读单清单(不存在 throw) |
| `Import(tarPath, name, desc?)` | 拷贝 + 生成 manifest(Validate: 时间戳非零) |
| `Remove(name)` | 删目录 |
| `Validate(manifest)` | name+arch 非空/createdAt≠0 |

## 兼容性事实（关键）
- **官方 .wsl tar 兼容导入**——官方发行版镜像=根内容 tar，我们安装流程=mkfs.btrfs→解包到 @（与文件系统无关）
- 我们**自有 rootfs 镜像**（导出当前根）——将来 `launch --image` 使用

## 边界
无压缩优化（v0.1 原样 tar.gz）；镜像与快照关系=互为材料（v0.2 打通 export）

## 测试归属
T7
