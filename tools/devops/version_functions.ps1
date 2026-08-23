Set-StrictMode -Version Latest

function Get-Current-Commit-Hash()
{
    return ([string](git log -1 --pretty=%h)).Trim()
}

function Get-VersionInfo
{
    [CmdletBinding()]
    param([Parameter(Mandatory = $true)]$Nightly)

    $ErrorActionPreference = "Stop"

    if ($Nightly)
    {
        $suffix = 'nightly'
    }
    else
    {
        $suffix = 'build'
    }

    $output = git.exe describe --tags --match *.*.* --abbrev=1
    if ($LastExitCode -ne 0)
    {
        # WSL-Plus: 无 tag 时 fallback（本地/CI 首次构建场景，标签未推送）
        # 输出对齐 git describe 结构: <tag(三段)>-<commitCount>-g<shortSha>（微软 tag 为三段，
        # version 解析: tag(三段) + commitCount → 四段版本过 PACKAGE_VERSION 校验）
        $shortSha = (git.exe rev-parse --short HEAD 2>$null).Trim()
        $output = "1.0.0-0-g" + $shortSha
        if ([string]::IsNullOrWhiteSpace($shortSha))
        {
            throw "git describe failed and fallback failed: $LastExitCode"
        }
    }

    $versionInfo = $output.split('-')
    if ($versionInfo.Length -lt 1)
    {
        throw "Unexpected output from git describe: $output"
    }

    $version = $versionInfo[0]
    if ($versionInfo.Length -lt 2)
    {
        $revision = 0 # This path is taken when the commit is directly on a tag
    }
    else
    {
        $revision = $versionInfo[1]
    }

    $result = @{
                'MsixVersion' = "$version.$revision"
                'NugetVersion' = "$version-$suffix{0:d3}" -f $revision
              }

    return $result
}
