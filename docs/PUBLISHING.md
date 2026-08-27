# WSL-Plus 发布/升级/回滚手册（PUBLISHING）

> 2026-08-25 · 分发铁律: **只用 exe + msi**（无 MSIX/MSIXbundle——BUILD_BUNDLE 不启用 + gluepackage 已禁）

## 1. 产物
```
bin\x64\Release\
  wsl.exe / wslservice.exe / wslc.exe / wslg.exe / wslhost.exe / wslrelay.exe / wslcsession.exe
  + 各 dll + wsl.msi（WiX 打包,同官方 UpgradeCode）
```

## 2. 升级关系（官方 → Plus）
```
MSI UpgradeCode 与官方相同(6D5B792B-...) → 同升级码 + 版本更高 = 无缝覆盖升级
规则:
  ① 用户装过【官方 MSI 版】  → 安装我们的 wsl.msi = 直接覆盖升级（数据/发行版保留）
  ② 用户装过【Store MSIX 版】→ 先 wsl --uninstall 卸载 Store 版 → 装我们的 MSI（互斥）
  ③ 我们后续发布新版本  → 版本号递增重新打包 → 直接覆盖旧 Plus
⚠️ 版本红线: PACKAGE_VERSION **必须 > 官方当前所有已发布版本 + 自家已发布版本**（当前线: 3.0.0.0 > 官方 2.9.9.0）;
  发布前 `gh api repos/microsoft/WSL/releases` 复核官方最新号——官方≥我们则大版本+1（fallback 1.0.0.0 仅编译期, 发布必须显式传）
```

## 3. 安装/升级命令
```
msiexec /i wsl.msi /q        （静默安装）
msiexec /i wsl.msi           （交互）
（覆盖安装同命令——旧版自动替换，注册表/发行版 VHDX 保留）
```

## 4. 回滚
```
回滚到官方版:
  ① 卸载 Plus: msiexec /x <ProductCode>（或 设置→应用卸载）
  ② 装官方版（下载 MSI / wsl --update 恢复其通道）
  （发行版数据不因卸载丢失——VHDX 与注册配置在 %USERPROFILE% 与 HKCU）
回滚到前一版 Plus: 直接装旧版 MSI（需版本号递减？MSI 不支持降——
  先卸载再装旧版; 或发布时同时产出旧代新号备份）
```

## 5. 官方更新覆盖我们的情况
```
微软发新官方 MSI 版本（用户手动装官方高版本）→ 覆盖 Plus → 恢复官方行为
对策: 重新合并上游 + 重建 Plus MSI（我们 CI/工作流已就绪）→ 再覆盖
（"永久 Plus"不存在——Plus 是活动构建,随我们流水线更新; 已写入开发准则）
```

## 6. 发布 Checklist
```
[ ] PACKAGE_VERSION 设置为高于当前所有已发布版本（版本红线!）
[ ] 全量编译通过（CI 全量轮绿）
[ ] 产物齐全: bin\x64\Release\wsl.exe + wsl.msi
[ ] 用真机 VM 行为验收（E7: 快照/克隆/网络/串口）
[ ] 本手册随版本归档（审计/资产管理目录留痕）
```
