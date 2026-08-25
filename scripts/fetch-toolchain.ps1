<#
.SYNOPSIS
    Resolve the prebuilt dependency toolchain for Windows.

.DESCRIPTION
    Downloads and verifies the pinned Windows vcpkg binary cache.

.PARAMETER Dest
    Where to extract archives. Defaults to a per-user cache directory.

.PARAMETER GitHubEnv
    Append US3_VCPKG_CACHE to $env:GITHUB_ENV (CI convenience).
    US3_REQUIRE_BINARY_CACHE is written alongside it.

.PARAMETER AllowMissing
    Exit successfully when the pin has no artifact.
#>

[CmdletBinding()]
param(
    [string]$Dest,
    [switch]$GitHubEnv,
    [switch]$AllowMissing
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$SourceDir = Split-Path -Parent $ScriptDir
$LockFile  = Join-Path $SourceDir 'buildsys\toolchain.lock.json'

if (-not (Test-Path $LockFile)) { throw "missing $LockFile" }

$Target = 'windows-x64'
$lock   = Get-Content -Raw $LockFile | ConvertFrom-Json

if (-not $lock.targets.PSObject.Properties.Name.Contains($Target)) {
    throw "no toolchain pin for target '$Target'"
}
$pin = $lock.targets.$Target

if ([string]::IsNullOrWhiteSpace($pin.sha256)) {
    if ($AllowMissing) { return }
    throw @"
Toolchain pin for '$Target' has no sha256 recorded.
Run the 'Toolchain' workflow to build and publish it, then merge the pin
update it produces.
"@
}

if (-not $Dest) {
    $Dest = if ($env:US3_TOOLCHAIN_DIR) { $env:US3_TOOLCHAIN_DIR }
            else { Join-Path $env:LOCALAPPDATA 'ultrascan3\toolchain' }
}

$CacheDir = Join-Path $Dest "$Target\$($pin.sha256)"
$Stamp    = Join-Path $CacheDir '.complete'

if (-not (Test-Path $Stamp)) {
    New-Item -ItemType Directory -Force -Path $Dest | Out-Null
    $TmpArchive = Join-Path $Dest ".$($pin.asset).partial"

    # Resolve the repository so forks use their own release assets.
    $repo = $env:GITHUB_REPOSITORY
    if (-not $repo) {
        $origin = (git -C $SourceDir remote get-url origin 2>$null)
        if ($origin) { $repo = $origin -replace '^git@[^:]+:', '' -replace '^https?://[^/]+/', '' -replace '\.git$', '' }
    }
    if (-not $repo) {
        throw "cannot determine the GitHub repository to download from. Set GITHUB_REPOSITORY=<owner>/<repo> and retry."
    }
    $url  = "https://github.com/$repo/releases/download/$($lock.release_tag)/$($pin.asset)"

    Write-Host "Fetching toolchain for $Target"
    Write-Host "  $url"

    $headers = @{}
    if ($env:GH_TOKEN) { $headers['Authorization'] = "Bearer $env:GH_TOKEN" }

    # Progress rendering significantly slows large non-interactive downloads.
    $oldProgress = $ProgressPreference
    $ProgressPreference = 'SilentlyContinue'
    try {
        Invoke-WebRequest -Uri $url -OutFile $TmpArchive -Headers $headers -MaximumRetryCount 5 -RetryIntervalSec 5
    }
    finally {
        $ProgressPreference = $oldProgress
    }

    # Verify the archive before extraction.
    $actual = (Get-FileHash -Path $TmpArchive -Algorithm SHA256).Hash.ToLower()
    if ($actual -ne $pin.sha256.ToLower()) {
        Remove-Item -Force $TmpArchive
        throw "checksum mismatch for $($pin.asset)`n  expected $($pin.sha256)`n  actual   $actual"
    }

    if (Test-Path $CacheDir) { Remove-Item -Recurse -Force $CacheDir }
    New-Item -ItemType Directory -Force -Path $CacheDir | Out-Null

    # bsdtar expects an unavailable `unzstd` helper; a native-command pipeline
    # would also corrupt binary data through PowerShell text decoding.
    $zstd = Get-Command zstd -ErrorAction SilentlyContinue
    if (-not $zstd) {
        throw "zstd was not found on PATH; it is required to unpack $($pin.asset)."
    }

    $TmpTar = "$TmpArchive.tar"
    & $zstd.Source -d -f -q $TmpArchive -o $TmpTar
    if ($LASTEXITCODE -ne 0) {
        Remove-Item -Force -ErrorAction SilentlyContinue $TmpTar
        throw "zstd failed to decompress $($pin.asset)"
    }

    & tar.exe -xf $TmpTar -C $CacheDir
    $TarExit = $LASTEXITCODE
    Remove-Item -Force -ErrorAction SilentlyContinue $TmpTar
    if ($TarExit -ne 0) { throw "extraction failed for $($pin.asset)" }

    Remove-Item -Force $TmpArchive
    New-Item -ItemType File -Path $Stamp -Force | Out-Null
    Write-Host "Toolchain extracted to $CacheDir"
}
else {
    Write-Host "Toolchain already present: $CacheDir"
}

if ($GitHubEnv -and $env:GITHUB_ENV) {
    Add-Content -Path $env:GITHUB_ENV -Value "US3_VCPKG_CACHE=$CacheDir"
    # Always false on Windows: every Windows pin is an archive target, so the
    # compiler comes from the runner image and can change under us. Emitted
    # rather than left unset so an inherited value cannot make build.ps1
    # enforce binary-only caching here. See fetch-toolchain.sh for the rule.
    Add-Content -Path $env:GITHUB_ENV -Value "US3_REQUIRE_BINARY_CACHE=false"
}

Write-Output $CacheDir
