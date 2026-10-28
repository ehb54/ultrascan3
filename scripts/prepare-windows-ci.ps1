#Requires -Version 5.1

<#
.SYNOPSIS
    Prepares a GitHub-hosted Windows runner for disk-intensive vcpkg builds.

.DESCRIPTION
    Removes large tool stacks that UltraScan3 does not use and places mutable
    vcpkg state on RUNNER_TEMP (normally D:). The vcpkg binary cache remains at
    its configured location so actions/cache can restore and save it.
#>

[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

if ($env:GITHUB_ACTIONS -ne "true") {
    Write-Host "Windows CI preparation skipped outside GitHub Actions."
    return
}

function Show-DiskState {
    Get-CimInstance Win32_LogicalDisk -Filter "DriveType=3" |
        Select-Object DeviceID,
            @{Name = "SizeGB"; Expression = { [Math]::Round($_.Size / 1GB, 1) }},
            @{Name = "FreeGB"; Expression = { [Math]::Round($_.FreeSpace / 1GB, 1) }} |
        Format-Table -AutoSize
}

Write-Host "=========================================="
Write-Host "Windows CI disk preflight"
Write-Host "=========================================="
Show-DiskState

# UltraScan3 uses the MSVC C++ toolchain, CMake, Ninja, Python, NSIS, Git, and
# NASM. These preinstalled stacks are unrelated and consume several GB on C:.
$CleanupCandidates = @(
    $env:ANDROID_HOME,
    $env:ANDROID_SDK_ROOT,
    "C:\Android",
    "C:\Program Files (x86)\Android",
    "C:\ghcup",
    "C:\Program Files\dotnet",
    "C:\Program Files\Microsoft SDKs\Azure",
    "C:\Program Files (x86)\Microsoft SDKs\Azure"
) | Where-Object { $_ } | Select-Object -Unique

# Build trees and vcpkg state live on RUNNER_TEMP's drive, so C: rarely needs
# reclaiming; the deletions below cost ~20 minutes when they do run.
$MinFreeGB = if ($env:US3_WINDOWS_MIN_FREE_GB) { [double]$env:US3_WINDOWS_MIN_FREE_GB } else { 40 }
$SystemFreeGB = (Get-CimInstance Win32_LogicalDisk -Filter "DeviceID='C:'").FreeSpace / 1GB

if ($SystemFreeGB -ge $MinFreeGB) {
    Write-Host ("Skipping runner cleanup: C: has {0:N1} GB free (threshold {1} GB)." -f $SystemFreeGB, $MinFreeGB)
} else {
    foreach ($Candidate in $CleanupCandidates) {
        if (-not (Test-Path -LiteralPath $Candidate)) { continue }

        Write-Host "Removing unused runner component: $Candidate"
        # rd deletes natively; Remove-Item -Recurse is far slower on trees with
        # many small files such as dotnet and ghcup.
        cmd /c "rd /s /q `"$Candidate`" >nul 2>&1"
        if (Test-Path -LiteralPath $Candidate) {
            # Runner images occasionally hold a file open. Partial cleanup still
            # provides useful headroom and must not obscure the actual build.
            Write-Warning "Could not completely remove $Candidate"
        }
    }
}

if (-not $env:RUNNER_TEMP) {
    throw "RUNNER_TEMP is not set on the GitHub Actions runner."
}

# Use short drive-rooted paths to keep Qt buildtrees below MAX_PATH.
$RunnerTempDrive = (Split-Path -Qualifier $env:RUNNER_TEMP)
if (-not $RunnerTempDrive) {
    throw "Could not determine the drive letter of RUNNER_TEMP ($env:RUNNER_TEMP)."
}

if (-not $env:US3_VCPKG_ROOT) {
    $env:US3_VCPKG_ROOT = "$RunnerTempDrive\v"
}
New-Item -ItemType Directory -Force -Path $env:US3_VCPKG_ROOT | Out-Null

$LegacyDownloads = Join-Path $HOME "vcpkg-downloads"
if (-not $env:US3_VCPKG_DOWNLOADS -or $env:US3_VCPKG_DOWNLOADS -eq $LegacyDownloads) {
    $env:US3_VCPKG_DOWNLOADS = "$RunnerTempDrive\d"
}

New-Item -ItemType Directory -Force -Path $env:US3_VCPKG_DOWNLOADS | Out-Null

$CacheDisplay = if ($env:US3_VCPKG_CACHE) { $env:US3_VCPKG_CACHE } else { "<build default>" }
Write-Host "Windows CI paths:"
Write-Host "  vcpkg root      : $env:US3_VCPKG_ROOT"
Write-Host "  vcpkg downloads : $env:US3_VCPKG_DOWNLOADS"
Write-Host "  vcpkg cache     : $CacheDisplay"
Write-Host "Disk after cleanup:"
Show-DiskState
