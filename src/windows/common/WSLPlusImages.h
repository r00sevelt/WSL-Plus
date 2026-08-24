/*++

Copyright (c) WSL-Plus contributors.

Module Name:

    WSLPlusImages.h

Abstract:

    WSL-Plus: 本地镜像库（D3）—— rootfs 镜像的私有存档/分发。
    存储: %USERPROFILE%\.wslplus\images\<name>\{rootfs.tar.gz, manifest.yaml}
    格式: 镜像 = 根内容 tar.gz + manifest（名/描述/日期/架构/基础版本）；
    与官方 .wsl tar 兼容导入（根内容解包与文件系统无关——安装流程 btrfs 化后照常）。

--*/

#pragma once

#include <filesystem>

namespace wsl::windows::common::wslplus::images
{
    struct ImageManifest
    {
        std::string name;
        std::string description;
        std::string architecture = "x64";
        std::string baseVersion;      // 基础发行版版本（如 ubuntu-24.04）
        uint32_t createdAtUtc = 0;    // Unix 时间戳

        bool operator==(const ImageManifest&) const = default;
    };

    std::filesystem::path ImagesRoot();

    // 库扫描（返回全部清单）
    std::vector<ImageManifest> List();

    // 从 tar 导入到库（官方 .wsl tar / 我们导出的 rootfs tar 均可）
    ImageManifest Import(_In_ const std::filesystem::path& tarPath, _In_ LPCWSTR name, _In_opt_ LPCWSTR description);

    // 从库删除（含内容目录）
    void Remove(_In_ LPCWSTR name);

    // 读取单镜像清单（不存在 → throw）
    ImageManifest Read(_In_ LPCWSTR name);

    // manifest 序列化校验（name 合法/日期非零）
    void Validate(const ImageManifest& manifest);
}
