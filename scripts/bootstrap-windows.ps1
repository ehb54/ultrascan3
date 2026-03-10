#Requires -Version 5.1
<#
.SYNOPSIS
    UltraScan3 Windows OS-level prerequisite installer.

.DESCRIPTION
    Installs the tools and components that must exist on a Windows machine
    BEFORE build.ps1 is run. This script does NOT build UltraScan3, does NOT
    invoke vcpkg, and does NOT duplicate any logic that belongs in build.ps1.

    The following are installed if missing:
      - Git
      - CMake
      - Ninja
      - NASM
      - Python 3 (for Sphinx documentation)
      - NSIS (Nullsoft installer, for --pkg builds)
      - Visual Studio Build Tools with the C++ Desktop workload

    Visual Studio is handled conservatively: if no qualifying VS installation
    is found the script prints clear remediation instructions. In CI it exits
    immediately (VS is always pre-installed on GitHub-hosted Windows runners).

    All winget installations are silent and non-interactive when CI=true.

.PARAMETER DryRun
    Print what would be installed without actually installing anything.

.PARAMETER Help
    Show this help message and exit.

.EXAMPLE
    .\scripts\bootstrap-windows.ps1
    Check and install all prerequisites interactively.

.EXAMPLE
    .\scripts\bootstrap-windows.ps1 -DryRun
    Show what would be installed without making any changes.

.NOTES
    Run via bootstrap.bat from any terminal (CMD or PowerShell):
        scripts\bootstrap.bat

    SUPPORTED PLATFORMS
        Windows 10 (build 19041+) / Windows 11
        Windows Server 2019 / 2022 / 2025  <-- GitHub Actions runner targets

    DESIGN PRINCIPLES
        - Idempotent: safe to run on a machine where tools are already present
        - Non-interactive when CI=true, prompts locally
        - Visual Studio install attempted locally only; CI fails clearly if absent
        - winget is the package manager of record (pre-installed on Win10 1809+)
        - vcvarsall.bat activation stays in build.ps1 (runtime, not bootstrap)
#>

param(
    [switch]$DryRun,
    [switch]$Help
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# =============================================================================
# HELP
# =============================================================================
if ($Help) {
    Get-Help $MyInvocation.MyCommand.Path -Full
    exit 0
}

# =============================================================================
# HELPERS
# =============================================================================
function Log   { param([string]$Msg) Write-Host "[bootstrap] $Msg" }
function Warn  { param([string]$Msg) Write-Host "[bootstrap] WARNING: $Msg" -ForegroundColor Yellow }
function Fatal { param([string]$Msg) Write-Host "[bootstrap] ERROR: $Msg" -ForegroundColor Red; exit 1 }

$NonInteractive = ($env:CI -eq "true")
if ($NonInteractive) { Log "Running in CI environment." }

# =============================================================================
# PLATFORM GUARD
# =============================================================================
if ($env:OS -ne "Windows_NT") {
    Fatal "This script is for Windows only.`nFor Linux use scripts/bootstrap-deps.sh, for macOS use scripts/bootstrap-macos.sh."
}

# =============================================================================
# WINGET CHECK
# =============================================================================
# winget ships with Windows 10 1809+ / Windows 11 and all GitHub-hosted
# Windows runners. If it is somehow absent we cannot auto-install anything,
# so fail clearly rather than silently skipping packages.
# =============================================================================
Log "Checking winget..."
if (-not (Get-Command winget -ErrorAction SilentlyContinue)) {
    Fatal "winget not found. It ships with Windows 10 1809 and later.`nInstall the App Installer package from the Microsoft Store, then re-run.`nSee: https://aka.ms/getwinget"
}
Log "winget is available: $(winget --version)"
Log ""

# =============================================================================
# PACKAGE DEFINITIONS
# =============================================================================
# Each entry is a hashtable with:
#   Id          winget package ID (exact, case-insensitive)
#   Binary      executable name to check with Get-Command (empty = always install)
#   Description human-readable name shown in output
#   CiRequired  if $true, fail in CI when missing rather than offering to install
#
# Groups mirror the pattern established in bootstrap-deps.sh and
# bootstrap-macos.sh so the three files are easy to maintain in parallel.

# --- Core build tools -------------------------------------------------------
# cmake: project build system (requires >= 3.21)
# git: vcpkg clones itself and its ports; also needed for source checkout
# ninja: the generator used by every CMake preset (see CMakePresets.json)
$PkgsToolchain = @(
    @{ Id = "Kitware.CMake";        Binary = "cmake";   Description = "CMake";  CiRequired = $true },
    @{ Id = "Git.Git";              Binary = "git";     Description = "Git";    CiRequired = $true },
    @{ Id = "Ninja-build.Ninja";    Binary = "ninja";   Description = "Ninja";  CiRequired = $true }
)

# --- Assembly ---------------------------------------------------------------
# nasm: vcpkg Qt and image format ports (libjpeg-turbo, openssl) require NASM
#       for optimised assembly routines on x64 and arm64
$PkgsAsm = @(
    @{ Id = "NASM.NASM"; Binary = "nasm"; Description = "NASM assembler"; CiRequired = $true }
)

# --- Python (Sphinx documentation) ------------------------------------------
# Python 3 is needed so build.ps1 can pip-install Sphinx from
# doc/manual/source/requirements.txt.  Documentation builds are treated as
# non-fatal (build.ps1 already warns when sphinx-build is absent) so this
# package is not CiRequired -- GitHub runners have Python pre-installed.
$PkgsPython = @(
    @{ Id = "Python.Python.3.13"; Binary = "python"; Description = "Python 3"; CiRequired = $false }
)

# --- NSIS installer builder -------------------------------------------------
# Required to produce the Windows .exe installer (--pkg flag in build.ps1).
# Not needed for plain builds, but bootstrapping it upfront avoids a surprise
# failure when someone first tries --pkg.
$PkgsNsis = @(
    @{ Id = "NSIS.NSIS"; Binary = "makensis"; Description = "NSIS installer builder"; CiRequired = $false }
)

# =============================================================================
# FULL PACKAGE LIST (NSIS binary overlaps with toolchain check -- deduplicated
# at install time via the Binary check)
# =============================================================================
$AllPkgs = $PkgsToolchain + $PkgsAsm + $PkgsPython + $PkgsNsis

# =============================================================================
# NSIS PATH SELF-HEAL
# makensis is often installed but not on PATH. Check standard locations and
# add to the session PATH before the presence check below so the binary check
# reflects reality rather than PATH gaps.
# =============================================================================
if (-not (Get-Command makensis -ErrorAction SilentlyContinue)) {
    foreach ($NsisDir in @("C:\Program Files (x86)\NSIS", "C:\Program Files\NSIS")) {
        if (Test-Path (Join-Path $NsisDir "makensis.exe")) {
            $env:PATH = "$NsisDir;$env:PATH"
            break
        }
    }
}

# =============================================================================
# DRY-RUN MODE
# =============================================================================
if ($DryRun) {
    Log "Dry-run mode -- nothing will be installed."
    Log ""
    Log "Packages that would be checked/installed:"
    foreach ($Pkg in $AllPkgs) {
        $Status = if (Get-Command $Pkg.Binary -ErrorAction SilentlyContinue) { "already installed" } else { "MISSING" }
        Log "  $($Pkg.Description.PadRight(30)) winget id: $($Pkg.Id)  [$Status]"
    }
    Log ""
    Log "Visual Studio: checked separately via vswhere (no winget install)"
    exit 0
}

# =============================================================================
# WINGET TOOL INSTALLATIONS
# =============================================================================
Log "Checking required tools..."
Log ""

$PkgsToInstall = @()
foreach ($Pkg in $AllPkgs) {
    $Found = $false
    if ($Pkg.Binary) {
        $Found = [bool](Get-Command $Pkg.Binary -ErrorAction SilentlyContinue)
    }
    if (-not $Found) {
        $PkgsToInstall += $Pkg
    }
}

# Deduplicate by Id in case NSIS appears twice
$PkgsToInstall = $PkgsToInstall | Sort-Object Id -Unique

if ($PkgsToInstall.Count -eq 0) {
    Log "All required tools are already installed."
    Log ""
} else {
    Log "Missing tools ($($PkgsToInstall.Count)):"
    foreach ($Pkg in $PkgsToInstall) {
        Log "  - $($Pkg.Description)"
    }
    Log ""

    # In CI, CiRequired packages must already be present on the runner image.
    # winget installs in CI are slow and may require PATH refresh / reboot.
    $CiMissing = $PkgsToInstall | Where-Object { $_.CiRequired }
    if ($NonInteractive -and $CiMissing.Count -gt 0) {
        Fatal "The following required tools are missing from the CI runner image:`n" +
              ($CiMissing | ForEach-Object { "  - $($_.Description) (winget: $($_.Id))" } | Out-String) +
              "`nEnsure the runner image or a prior workflow step installs these tools."
    }

    if (-not $NonInteractive) {
        $Answer = Read-Host "[bootstrap] Install missing tools via winget? [y/N]"
        if ($Answer -notmatch '^[Yy]$') {
            Log "Aborted. No tools were installed."
            exit 0
        }
    }

    foreach ($Pkg in $PkgsToInstall) {
        Log "Installing $($Pkg.Description) ($($Pkg.Id))..."
        $WingetArgs = @(
            "install", "--exact", "--id", $Pkg.Id,
            "--accept-package-agreements", "--accept-source-agreements",
            "--silent"
        )
        winget @WingetArgs
        if ($LASTEXITCODE -ne 0 -and $LASTEXITCODE -ne -1978335189) {
            # -1978335189 (0x8A150007) = APPINSTALLER_CLI_ERROR_PACKAGE_ALREADY_INSTALLED
            # winget returns this when a newer version is already present; treat as success.
            Warn "$($Pkg.Description) install returned exit code $LASTEXITCODE -- may need PATH refresh or reboot."
        } else {
            Log "$($Pkg.Description) installed."
        }
    }

    # Refresh PATH for the current session so newly installed binaries are
    # immediately available without requiring a new terminal window.
    $env:PATH = [System.Environment]::GetEnvironmentVariable("PATH", "Machine") + ";" +
                [System.Environment]::GetEnvironmentVariable("PATH", "User")

    # Re-add NSIS if it still isn't on the refreshed PATH
    if (-not (Get-Command makensis -ErrorAction SilentlyContinue)) {
        foreach ($NsisDir in @("C:\Program Files (x86)\NSIS", "C:\Program Files\NSIS")) {
            if (Test-Path (Join-Path $NsisDir "makensis.exe")) {
                $env:PATH = "$NsisDir;$env:PATH"
                break
            }
        }
    }

    Log ""
}

# =============================================================================
# VISUAL STUDIO CHECK
# =============================================================================
# vcpkg and CMake on Windows require MSVC. We check for a VS installation with
# the "Desktop development with C++" workload via vswhere.
#
# We do NOT attempt to install VS via winget here because:
#   1. It takes 15-30 minutes and may require a reboot.
#   2. The winget VS package does not yet support --add workload flags reliably.
#   3. GitHub-hosted Windows runners always have VS pre-installed.
#
# Instead: detect presence, print clear instructions if absent, and in CI fail
# loudly so the runner image misconfiguration is caught immediately.
# =============================================================================
Log "Checking Visual Studio..."

$VsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

if (-not (Test-Path $VsWhere)) {
    $VsMsg = @"
Visual Studio (with 'Desktop development with C++' workload) is required.
vswhere.exe was not found, which means no Visual Studio is installed.

Install Visual Studio 2022 Community (free) from:
  https://visualstudio.microsoft.com/downloads/

During installation, select the workload:
  'Desktop development with C++'

Or install silently via winget (then re-run this script):
  winget install --id Microsoft.VisualStudio.2022.Community --silent `
    --override "--quiet --add Microsoft.VisualStudio.Workload.NativeDesktop --includeRecommended"
"@
    if ($NonInteractive) {
        Fatal $VsMsg
    } else {
        Write-Host ""
        Write-Host $VsMsg -ForegroundColor Yellow
        Write-Host ""
        Fatal "Visual Studio not found. Install it and re-run bootstrap."
    }
}

# vswhere is present -- find a qualifying installation
$VsPath = & $VsWhere -latest `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    -property installationPath 2>$null

if (-not $VsPath) {
    $VsMsg = @"
Visual Studio is installed but the 'Desktop development with C++' workload
(component: Microsoft.VisualStudio.Component.VC.Tools.x86.x64) was not found.

Open the Visual Studio Installer and add the workload:
  'Desktop development with C++'

Or add the component from the command line:
  & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vs_installer.exe" `
      modify --installPath "<VS install path>" `
      --add Microsoft.VisualStudio.Workload.NativeDesktop --quiet
"@
    if ($NonInteractive) {
        Fatal $VsMsg
    } else {
        Write-Host ""
        Write-Host $VsMsg -ForegroundColor Yellow
        Write-Host ""
        Fatal "C++ workload missing. Add it via the Visual Studio Installer and re-run."
    }
}

$VsVersion = & $VsWhere -latest `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    -property catalog_productDisplayVersion 2>$null

Log "Visual Studio $VsVersion found."
Log "  at: $VsPath"
Log ""

# =============================================================================
# POST-INSTALL VERIFICATION
# =============================================================================
Log "Verifying key tools on PATH..."

$VerifyTools = @("cmake", "git", "ninja", "nasm", "python")
$MissingAfter = @()
foreach ($Tool in $VerifyTools) {
    if (-not (Get-Command $Tool -ErrorAction SilentlyContinue)) {
        $MissingAfter += $Tool
    }
}

if ($MissingAfter.Count -gt 0) {
    Warn "The following tools are still not on PATH after installation:"
    foreach ($Tool in $MissingAfter) {
        Warn "  $Tool"
    }
    Warn ""
    Warn "This usually means a PATH refresh is needed. Options:"
    Warn "  1. Close this terminal and open a new one, then re-run build.bat"
    Warn "  2. Or reboot if a winget install required it"
    Warn "Build may fail. Please verify before running build.bat."
} else {
    Log "All key tools verified on PATH."
}

# =============================================================================
# DONE
# =============================================================================
Log ""
Log "Bootstrap complete."
Log ""
Log "Next steps:"
Log "  1. Run 'scripts\build.bat --help' to see build options."
Log "  2. First build: 'scripts\build.bat'  (Qt6, APP profile)"
Log "     (vcpkg will build all library dependencies; allow 30-45 minutes)"
