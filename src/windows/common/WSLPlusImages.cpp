/*++

Copyright (c) WSL-Plus contributors.

Module Name:

    WSLPlusImages.cpp

Abstract:

    WSL-Plus: 本地镜像库实现（D3）。

--*/

#include "precomp.h"
#include <shlobj_core.h> // FOLDERID_* 唯一定义处
#include "WSLPlusImages.h"
#include <yaml-cpp/yaml.h>

namespace wsl::windows::common::wslplus::images
{
namespace
{
    constexpr const wchar_t* kImagesDir = L"images";
    constexpr const char* kRootfsFile = "rootfs.tar.gz";
    constexpr const char* kManifestFile = "manifest.yaml";
}

std::filesystem::path ImagesRoot()
{
    auto home = wsl::windows::common::filesystem::GetKnownFolderPath(FOLDERID_UserProfile);
    return home / L".wslplus" / kImagesDir;
}

std::vector<ImageManifest> List()
{
    std::vector<ImageManifest> result;
    const auto root = ImagesRoot();
    if (!std::filesystem::exists(root))
    {
        return result;
    }

    for (const auto& entry : std::filesystem::directory_iterator(root))
    {
        if (!entry.is_directory())
        {
            continue;
        }
        const auto manifestPath = entry.path() / kManifestFile;
        if (!std::filesystem::exists(manifestPath))
        {
            continue;
        }
        try
        {
            result.emplace_back(Read(entry.path().filename().c_str()));
        }
        CATCH_LOG()
    }
    return result;
}

void Validate(const ImageManifest& manifest)
{
    if (manifest.name.empty() || manifest.architecture.empty())
    {
        THROW_HR(E_INVALIDARG);
    }
    if (manifest.createdAtUtc == 0)
    {
        THROW_HR(E_INVALIDARG);
    }
}

ImageManifest Read(_In_ LPCWSTR name)
{
    const auto manifestPath = ImagesRoot() / name / kManifestFile;
    THROW_HR_IF(HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND), !std::filesystem::exists(manifestPath));

    const auto node = YAML::LoadFile(manifestPath.string());
    ImageManifest manifest;
    manifest.name = node["name"].as<std::string>("");
    manifest.description = node["description"].as<std::string>("");
    manifest.architecture = node["architecture"].as<std::string>("x64");
    manifest.baseVersion = node["base-version"].as<std::string>("");
    manifest.createdAtUtc = node["created-at"].as<uint32_t>(0);
    return manifest;
}

ImageManifest Import(_In_ const std::filesystem::path& tarPath, _In_ LPCWSTR name, _In_opt_ LPCWSTR description)
{
    THROW_HR_IF(E_INVALIDARG, !std::filesystem::exists(tarPath));

    const std::wstring imageName(name);
    auto destDir = ImagesRoot() / imageName;
    std::filesystem::create_directories(destDir);

    // 复制 rootfs 内容
    const auto destTar = destDir / kRootfsFile;
    THROW_LAST_ERROR_IF(!std::filesystem::copy_file(tarPath, destTar, std::filesystem::copy_options::overwrite_existing));

    ImageManifest manifest;
    manifest.name = wsl::windows::common::string::WideToMultiByte(imageName);
    manifest.description = description ? wsl::windows::common::string::WideToMultiByte(description) : "";
    manifest.architecture = "x64";
    manifest.createdAtUtc = static_cast<uint32_t>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    Validate(manifest);

    YAML::Node node;
    node["name"] = manifest.name;
    node["description"] = manifest.description;
    node["architecture"] = manifest.architecture;
    node["base-version"] = manifest.baseVersion;
    node["created-at"] = manifest.createdAtUtc;
    std::ofstream out(destDir / kManifestFile, std::ios::trunc);
    THROW_LAST_ERROR_IF(!out);
    out << node;

    return manifest;
}

void Remove(_In_ LPCWSTR name)
{
    const auto dir = ImagesRoot() / name;
    THROW_HR_IF(HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND), !std::filesystem::exists(dir));
    std::filesystem::remove_all(dir);
}
} // namespace wsl::windows::common::wslplus::images
