# WSL-Plus Preflight check (run before ANY commit/push - catches known failure patterns)
# Usage: powershell -File tools\WSLPlus-Preflight.ps1
# Read-only. Zero cloud. Zero memory pressure.
$ErrorActionPreference = "Stop"
$repo = Split-Path $PSScriptRoot -Parent
$problems = @()

# Check 1: try / CATCH_RETURN pairing (IFACEMETHOD region)
function Check-TryCatch($file, $label) {
    if (-not (Test-Path $file)) { return }
    $content = Get-Content $file -Raw
    $tryCount = ([regex]::Matches($content, '\)\s*try\s*\{')).Count
    $catchCount = ([regex]::Matches($content, 'CATCH_RETURN\(')).Count
    if ($tryCount -ne $catchCount) {
        $problems += "$label : try=$tryCount but CATCH_RETURN=$catchCount (FIX BEFORE PUSH)"
    } else {
        Write-Host "OK   $label : try/catch paired ($tryCount)" -ForegroundColor Green
    }
}
Check-TryCatch "$repo\src\windows\service\exe\LxssUserSession.cpp" "LxssUserSession.cpp"

# Check 2: forbidden invented symbols (past incidents)
$forbidden = @("FindOrCreateUserSession", "ToNarrow", "PortRelaySpec", "RestoreMarkerFile")
foreach ($f in $forbidden) {
    $hits = Get-ChildItem "$repo\src" -Recurse -Filter "*.cpp" | Select-String -Pattern $f -SimpleMatch
    if ($hits) { $problems += "forbidden symbol '$f' at $($hits[0].Path):$($hits[0].LineNumber)" }
}

# Check 3: FOLDERID_ raw symbol (old pattern - use GetUserProfileDirectoryW)
foreach ($f in @("WSLPlusNetworks.cpp","WSLPlusImages.cpp","WSLPlusDevices.cpp")) {
    $p = "$repo\src\windows\common\$f"
    if (Test-Path $p) {
        $h = Select-String -Path $p -Pattern "FOLDERID_" -SimpleMatch
        if ($h) { $problems += "$f : raw FOLDERID_ symbol (use GetUserProfileDirectoryW pattern)" }
    }
}

# Check 4: bare argv passed to narrow-interface module calls
# NOTE: images::Remove/Import are WIDE (LPCWSTR) - excluded. Narrow set: networks/devices only.
$callFiles = @("WSLPlusCommands.cpp","WSLPlusNetworks.cpp","WSLPlusDevices.cpp")
foreach ($f in $callFiles) {
    $p = "$repo\src\windows\common\$f"
    if (Test-Path $p) {
        $h = Select-String -Path $p -Pattern "SetPolicy\(\s*argv|Remove\(\s*argv|Attach\(\s*argv|Attachments\(\s*argv"
        if ($h) { foreach ($m in $h) { $problems += "${f}:$($m.LineNumber) : bare argv to narrow interface (use Narrow())" } }
    }
}

Write-Host ""
if ($problems.Count -eq 0) {
    Write-Host "SUCCESS: preflight passed - safe to commit/push" -ForegroundColor Green
    exit 0
} else {
    Write-Host "FAILED: $($problems.Count) issue(s) - fix locally before pushing:" -ForegroundColor Red
    foreach ($p in $problems) { Write-Host "  - $p" -ForegroundColor Red }
    exit 1
}
