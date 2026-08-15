#Requires -Version 5.1

<#
.SYNOPSIS
    Builds one stage of the Windows Qt6 vcpkg binary cache.

.DESCRIPTION
    The workflow invokes this script sequentially on a dedicated GitHub-hosted
    runner. Each invocation builds one manifest feature into the shared binary
    cache. The workflow publishes the final cache only after all stages pass.
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateRange(1, 5)]
    [int]$Stage
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = (Resolve-Path (Join-Path $ScriptDir "..")).Path

$Feature = switch ($Stage) {
    1 { $null }
    2 { "warm-qt6-stage2" }
    3 { "warm-qt6-stage3" }
    4 { "warm-qt6-stage4" }
    5 { "warm-qt6-stage5" }
}

& (Join-Path $ScriptDir "prepare-windows-ci.ps1")

& (Join-Path $ScriptDir "bootstrap-windows.ps1")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$VcpkgRoot = $env:US3_VCPKG_ROOT
$VcpkgCache = if ($env:US3_VCPKG_CACHE) {
    $env:US3_VCPKG_CACHE
} else {
    Join-Path $HOME ".vcpkg-cache"
}
$VcpkgDownloads = $env:US3_VCPKG_DOWNLOADS
$InstallRoot = Join-Path $env:RUNNER_TEMP "us3-windows\vcpkg-installed-warm"

New-Item -ItemType Directory -Force -Path $VcpkgCache, $VcpkgDownloads, $InstallRoot | Out-Null

if (-not (Test-Path (Join-Path $VcpkgRoot ".git"))) {
    Write-Host "Cloning vcpkg into $VcpkgRoot"
    git clone https://github.com/microsoft/vcpkg.git $VcpkgRoot
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

$Manifest = Get-Content (Join-Path $RepoRoot "vcpkg.json") -Raw | ConvertFrom-Json
$Baseline = $Manifest.'builtin-baseline'
if (-not $Baseline) { throw "vcpkg.json does not define builtin-baseline." }

git -C $VcpkgRoot cat-file -e "$Baseline^{commit}" 2>$null
if ($LASTEXITCODE -ne 0) {
    Write-Host "Fetching vcpkg baseline $Baseline"
    git -C $VcpkgRoot fetch --no-tags origin $Baseline
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

$VcpkgExe = Join-Path $VcpkgRoot "vcpkg.exe"
if (-not (Test-Path $VcpkgExe)) {
    Write-Host "Bootstrapping vcpkg"
    Push-Location $VcpkgRoot
    try {
        & .\bootstrap-vcpkg.bat -disableMetrics
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    }
    finally {
        Pop-Location
    }
}

$env:VCPKG_ROOT = $VcpkgRoot
$env:VCPKG_DOWNLOADS = $VcpkgDownloads
$env:VCPKG_BINARY_SOURCES = "clear;files,$VcpkgCache,readwrite"
$env:VCPKG_MAX_CONCURRENCY = if ($env:US3_BUILD_JOBS) { $env:US3_BUILD_JOBS } else { "4" }

$Arguments = @(
    "install",
    "--triplet=x64-windows",
    "--host-triplet=x64-windows",
    "--overlay-triplets=$(Join-Path $RepoRoot 'admin\cmake\triplets')",
    "--overlay-ports=$(Join-Path $RepoRoot 'buildsys\vcpkg\overlay-ports')",
    "--x-install-root=$InstallRoot",
    "--x-no-default-features",
    "--clean-after-build"
)
if ($Feature) { $Arguments += "--x-feature=$Feature" }

$FeatureDisplay = if ($Feature) { $Feature } else { "<base dependencies>" }
Write-Host "=========================================="
Write-Host "Windows vcpkg cache warm stage $Stage / 5"
Write-Host "=========================================="
Write-Host "Feature   : $FeatureDisplay"
Write-Host "Root      : $VcpkgRoot"
Write-Host "Install   : $InstallRoot"
Write-Host "Downloads : $VcpkgDownloads"
Write-Host "Cache     : $VcpkgCache"

Push-Location $RepoRoot
try {
    & $VcpkgExe @Arguments
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}
finally {
    Pop-Location
}

$Archives = @(Get-ChildItem -LiteralPath $VcpkgCache -Recurse -File -ErrorAction SilentlyContinue)
$CacheBytes = ($Archives | Measure-Object -Property Length -Sum).Sum
if (-not $CacheBytes) { $CacheBytes = 0 }

Write-Host "Warm stage $Stage completed: $($Archives.Count) cache files, $([Math]::Round($CacheBytes / 1MB, 1)) MB"
if ($Archives.Count -eq 0 -or $CacheBytes -lt 1MB) {
    throw "Warm stage completed without producing a usable vcpkg binary cache."
}
