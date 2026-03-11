#Requires -Version 5.1
<#
.SYNOPSIS
    UltraScan3 Windows build script.

.DESCRIPTION
    Configures and builds UltraScan3 using CMake presets and vcpkg.
    Uses the same arguments as build.sh on macOS/Linux.

.PARAMETER rebuild
    Tier 1: Remove the CMake build directory only. vcpkg packages are untouched
    and restored from binary cache. Use when the build tree is corrupted or you
    want a clean UltraScan recompile without touching dependencies.

.PARAMETER clean
    Tier 2: Remove build dir + vcpkg buildtrees + installed packages for the
    active triplet. Forces vcpkg to reinstall all dependencies. Required after
    vcpkg.json feature changes (e.g. adding qt5-tools[assistant]).

.PARAMETER purge-cache
    Tier 3 (additive to -clean): also wipes the binary cache, forcing a full
    recompile from source. Use when switching compilers or suspecting cache
    corruption. Has no effect without -clean.

.PARAMETER qt6
    Build with Qt6 + Qwt6.3.0 [default]

.PARAMETER qt5-qwt616
    Build with Qt5 + Qwt6.1.6

.PARAMETER qt5-qwt630
    Build with Qt5 + Qwt6.3.0

.PARAMETER arch
    Target architecture: x64 (default, auto-detect) or arm64

.PARAMETER profile
    Build profile: APP (default), TEST, HPC

.PARAMETER vcpkg-root
    Path to vcpkg installation. Priority: --vcpkg-root > US3_VCPKG_ROOT env > source-tree vcpkg > ~/vcpkg

.PARAMETER pkg
    Build the Windows NSIS installer after compiling

.PARAMETER help
    Show this help message

.EXAMPLE
    .\build.bat
    Qt6, auto-detect arch, APP profile

.EXAMPLE
    .\build.bat --arch arm64
    Qt6, ARM64, APP profile

.EXAMPLE
    .\build.bat --qt5-qwt616 --arch arm64 TEST
    Qt5 + Qwt6.1.6, ARM64, TEST profile

.EXAMPLE
    .\build.bat --rebuild
    Wipe build dir only, rebuild UltraScan (vcpkg untouched)

.EXAMPLE
    .\build.bat --clean
    Full dep reinstall (after vcpkg.json feature changes)

.EXAMPLE
    .\build.bat --clean --purge-cache
    Nuke everything, recompile all deps from source

.EXAMPLE
    .\build.bat --vcpkg-root C:\dev\myrepo\vcpkg
    Use a specific vcpkg installation

.EXAMPLE
    .\build.bat --pkg
    Build and produce the Windows NSIS installer

.NOTES
    Run via build.bat from any terminal - no special environment needed:
        scripts\build.bat
        scripts\build.bat --arch arm64
        scripts\build.bat --rebuild
        scripts\build.bat --clean --qt5-qwt616 --arch arm64 TEST
        scripts\build.bat --vcpkg-root C:\dev\vcpkg
        scripts\build.bat --pkg

    VCPKG LOCATION (in order of priority):
        1. --vcpkg-root argument
        2. US3_VCPKG_ROOT environment variable
        3. vcpkg\ directory inside the source tree
        4. $HOME\vcpkg (default)

    ENVIRONMENT VARIABLES:
        US3_BUILD_JOBS      Override number of parallel build jobs
        US3_VCPKG_ROOT      Override vcpkg location (see priority above)
        US3_VCPKG_CACHE     Override binary cache path (default: $HOME\.vcpkg-cache)
#>

param(
    [Parameter(Position = 0)]
    [ValidateSet("APP", "TEST", "HPC")]
    [string]$profile = "APP",

    [switch]${rebuild},
    [switch]${clean},
    [switch]${purge-cache},
    [switch]${pkg},
    [switch]${qt6},
    [switch]${qt5-qwt616},
    [switch]${qt5-qwt630},
    [string]${arch}       = "",
    [string]${vcpkg-root} = "",
    [switch]${help}
)

$DocsBuilt = $true
$DocsStatusMessage = ""
$profile = $profile.ToUpperInvariant()

# =============================================================================
# HELP
# =============================================================================
if (${help}) {
    Write-Host "Usage: build.bat [OPTIONS] [PROFILE]"
    Write-Host ""
    Write-Host "OPTIONS:"
    Write-Host "  --rebuild                Tier 1: removes the CMake build directory only."
    Write-Host "                             Fast - vcpkg packages untouched, restored from binary cache."
    Write-Host "                             Use when the build tree is corrupted or you want a"
    Write-Host "                             clean UltraScan recompile without touching dependencies."
    Write-Host "  --clean                  Tier 2: removes build dir + vcpkg buildtrees + installed"
    Write-Host "                             packages for the active triplet. Forces vcpkg to reinstall"
    Write-Host "                             all dependencies. Required after vcpkg.json feature changes."
    Write-Host "  --clean --purge-cache    Tier 3: same as --clean plus wipes the binary cache"
    Write-Host "                             (~/.vcpkg-cache). Forces full recompile from source."
    Write-Host "                             Use when switching compilers or suspecting cache corruption."
    Write-Host "  --pkg                    Build the Windows NSIS installer"
    Write-Host "  --qt6                    Build with Qt6 + Qwt6.3.0 [default]"
    Write-Host "  --qt5-qwt616             Build with Qt5 + Qwt6.1.6"
    Write-Host "  --qt5-qwt630             Build with Qt5 + Qwt6.3.0"
    Write-Host "  --arch x64               Target x64 architecture [default: auto-detect]"
    Write-Host "  --arch arm64             Target ARM64 architecture"
    Write-Host "  --vcpkg-root <path>      Path to vcpkg installation"
    Write-Host "  --help                   Show this help message"
    Write-Host ""
    Write-Host "PROFILE:"
    Write-Host "  APP                      Desktop/user build (GUI + programs + DB) [default]"
    Write-Host "  TEST                     Dev/CI build (programs + tests, prefer static libs)"
    Write-Host "  HPC                      Headless / no DB / no GUI"
    Write-Host ""
    Write-Host "VCPKG LOCATION (in order of priority):"
    Write-Host "  1. --vcpkg-root <path> argument"
    Write-Host "  2. US3_VCPKG_ROOT environment variable"
    Write-Host "  3. vcpkg\ directory inside the source tree"
    Write-Host "  4. `$HOME\vcpkg (default)"
    Write-Host ""
    Write-Host "EXAMPLES:"
    Write-Host "  build.bat                                  # Qt6, auto-detect arch, APP"
    Write-Host "  build.bat --arch arm64                     # Qt6, ARM64, APP"
    Write-Host "  build.bat --qt5-qwt616                     # Qt5 + Qwt6.1.6, APP"
    Write-Host "  build.bat --qt6 TEST                       # Qt6, TEST profile"
    Write-Host "  build.bat --rebuild                        # Wipe build dir, rebuild UltraScan only"
    Write-Host "  build.bat --clean                          # Full dep reinstall (after vcpkg.json changes)"
    Write-Host "  build.bat --clean --purge-cache            # Nuke everything, recompile deps from source"
    Write-Host "  build.bat --clean --arch arm64 TEST        # Clean ARM64 Qt6 TEST build"
    Write-Host "  build.bat --vcpkg-root C:\dev\vcpkg        # Use specific vcpkg"
    Write-Host "  build.bat --vcpkg-root .\vcpkg             # Use source-tree vcpkg"
    Write-Host "  build.bat --pkg                            # Build + produce NSIS installer"
    Write-Host ""
    Write-Host "ENVIRONMENT VARIABLES:"
    Write-Host "  US3_BUILD_JOBS           Override number of parallel build jobs"
    Write-Host "  US3_VCPKG_ROOT           Override vcpkg location (see priority above)"
    Write-Host "  US3_VCPKG_CACHE          Override binary cache path (default: `$HOME\.vcpkg-cache)"
    exit 0
}

# =============================================================================
# RESOLVE QT VERSION
# =============================================================================
$QtSuffix = "-qt6"
if (${qt5-qwt616}.IsPresent) { $QtSuffix = "-qt5-qwt616" }
elseif (${qt5-qwt630}.IsPresent) { $QtSuffix = "-qt5-qwt630" }

$profile = $profile.ToUpperInvariant()

if (@("APP","TEST","HPC") -notcontains $profile) {
    Write-Host "ERROR: --profile must be APP, TEST, or HPC" -ForegroundColor Red
    exit 1
}

# =============================================================================
# ARCHITECTURE
# =============================================================================
$Arch = ${arch}
if (-not $Arch) {
    $CpuArch = (Get-CimInstance Win32_Processor).Architecture
    if ($CpuArch -eq 12) { $Arch = "arm64" } else { $Arch = "x64" }
    Write-Host "Auto-detected architecture: $Arch"
} else {
    if (@("x64","arm64") -notcontains $Arch) {
        Write-Host "ERROR: --arch must be x64 or arm64" -ForegroundColor Red
        exit 1
    }
}

# =============================================================================
# BUILD PRESET
# =============================================================================
if ($Arch -eq "arm64") {
    $Preset = "windows-release$QtSuffix-arm64"
} else {
    $Preset = "windows-release$QtSuffix"
}

# =============================================================================
# RESOLVE SOURCE ROOT
# Defined early so bootstrap-windows.ps1 can be located relative to this script.
# =============================================================================
$ScriptDir  = Split-Path -Parent $MyInvocation.MyCommand.Path
$SourceRoot = (Resolve-Path (Join-Path $ScriptDir "..")).Path
Set-Location $SourceRoot

# =============================================================================
# BUILD DIRECTORY
# =============================================================================
$BuildDir = Join-Path $SourceRoot "build\$Preset"

# =============================================================================
# HEADER
# =============================================================================
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "UltraScan3 Build Setup (Windows)"          -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "NOTE: First build takes 30-45 minutes."
Write-Host "      Dependencies are cached after that."
Write-Host ""

$NonInteractive = ($env:CI -eq "true")
if ($NonInteractive) { Write-Host "Running in CI environment" -ForegroundColor Yellow }

Write-Host "Selected build profile : ${profile}"
$QtLabel = switch ($QtSuffix) {
    "-qt6"        { "Qt6 (Qwt 6.3.0)" }
    "-qt5-qwt616" { "Qt5 (Qwt 6.1.6)" }
    "-qt5-qwt630" { "Qt5 (Qwt 6.3.0)" }
    default       { $QtSuffix }
}
Write-Host "Selected Qt variant    : $QtLabel"
Write-Host "Architecture           : $Arch"
Write-Host "Preset                 : $Preset"
if (${rebuild} -and -not ${clean}) { Write-Host "Rebuild requested       : --rebuild (build dir only)" -ForegroundColor Yellow }
if (${clean})       { Write-Host "Clean requested         : --clean (build dir + vcpkg installed/ for triplet)" -ForegroundColor Yellow }
if (${purge-cache}) { Write-Host "Purge cache requested   : --purge-cache (binary cache will also be wiped)" -ForegroundColor Yellow }
if (${pkg})         { Write-Host "Installer requested     : --pkg" -ForegroundColor Cyan }
Write-Host ""

# =============================================================================
# BUILD PARALLELISM
# =============================================================================
$Cores = if ($env:NUMBER_OF_PROCESSORS) { [int]$env:NUMBER_OF_PROCESSORS } else { 4 }
if ($env:US3_BUILD_JOBS) {
    # Explicit override always wins
    $BuildJobs = [int]$env:US3_BUILD_JOBS
} elseif ($env:CI -eq "true") {
    # CI runners are dedicated -- use every core
    $BuildJobs = $Cores
} else {
    # Local builds: leave ~10% headroom to keep the machine usable
    $BuildJobs = [Math]::Max(1, [Math]::Floor($Cores * 0.9))
}

Write-Host "Detected $Cores cores; using $BuildJobs parallel build jobs."
Write-Host ""

$env:VCPKG_MAX_CONCURRENCY = "$BuildJobs"

# =============================================================================
# WINDOWS BOOTSTRAP
# Delegates to bootstrap-windows.ps1, which is the single authoritative source
# for winget-level prerequisites (cmake, git, ninja, nasm, python, nsis) and
# Visual Studio presence verification. It is idempotent: exits immediately with
# no side effects when all tools are already installed (typical on repeat runs).
# After bootstrap completes, the NSIS PATH self-heal below ensures makensis is
# on PATH for this session even when NSIS is freshly installed.
# =============================================================================
$BootstrapScript = Join-Path $ScriptDir "bootstrap-windows.ps1"
if (-not (Test-Path $BootstrapScript)) {
    Write-Host "ERROR: bootstrap-windows.ps1 not found at $BootstrapScript" -ForegroundColor Red
    Write-Host "Please ensure scripts\bootstrap-windows.ps1 exists in the repository."
    exit 1
}

& $BootstrapScript
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Write-Host ""

# =============================================================================
# NSIS PATH SELF-HEAL
# makensis is often installed but not on the default PATH (NSIS does not add
# itself on all Windows versions). Check standard install locations and add
# whichever one exists to the session PATH so downstream cmake --build can
# find it when invoking the package_windows_nsis target.
# bootstrap-windows.ps1 performs the same check during install; this handles
# the case where NSIS was already installed before bootstrap ran.
# =============================================================================
if (-not (Get-Command makensis -ErrorAction SilentlyContinue)) {
    foreach ($NsisDir in @("C:\Program Files (x86)\NSIS", "C:\Program Files\NSIS")) {
        if (Test-Path (Join-Path $NsisDir "makensis.exe")) {
            Write-Host "Found NSIS at $NsisDir -- adding to PATH for this session." -ForegroundColor Yellow
            $env:PATH = "$NsisDir;$env:PATH"
            break
        }
    }
}

# =============================================================================
# SPHINX CHECK
# bootstrap-windows.ps1 ensures python is installed. Here we attempt to install
# Sphinx via pip if sphinx-build is still absent, matching the behaviour of
# build.sh on Linux/macOS.
# =============================================================================
if (-not (Get-Command sphinx-build -ErrorAction SilentlyContinue)) {
    $DocsBuilt = $false
    $SphinxRequirements = Join-Path $SourceRoot "doc\manual\source\requirements.txt"
    if (Test-Path $SphinxRequirements) {
        Write-Host "sphinx-build not found -- attempting pip install from requirements.txt..." -ForegroundColor Yellow
        $PyCmd = if (Get-Command py -ErrorAction SilentlyContinue) { "py" }
                 elseif (Get-Command python -ErrorAction SilentlyContinue) { "python" }
                 else { $null }
        if ($PyCmd) {
            & $PyCmd -m pip install -q -r $SphinxRequirements 2>$null
            if (Get-Command sphinx-build -ErrorAction SilentlyContinue) {
                Write-Host "sphinx-build installed successfully." -ForegroundColor Green
                $DocsBuilt = $true
            } else {
                $DocsStatusMessage = "Documentation not built: sphinx-build not found after pip install."
                Write-Host "WARNING: sphinx-build still not found. Help files will not be generated." -ForegroundColor Yellow
            }
        } else {
            $DocsStatusMessage = "Documentation not built: Python not found."
            Write-Host "WARNING: Python not found. Install Python then run:" -ForegroundColor Yellow
            Write-Host "  py -m pip install -r doc\manual\source\requirements.txt" -ForegroundColor Yellow
        }
    } else {
        $DocsStatusMessage = "Documentation not built: sphinx-build not found."
    }
    Write-Host ""
} else {
    # sphinx-build present -- silently ensure requirements are up to date
    $SphinxRequirements = Join-Path $SourceRoot "doc\manual\source\requirements.txt"
    if (Test-Path $SphinxRequirements) {
        $PyCmd = if (Get-Command py -ErrorAction SilentlyContinue) { "py" }
                 elseif (Get-Command python -ErrorAction SilentlyContinue) { "python" }
                 else { $null }
        if ($PyCmd) { & $PyCmd -m pip install -q -r $SphinxRequirements 2>$null }
    }
    Write-Host "sphinx-build is available: $(Get-Command sphinx-build | Select-Object -ExpandProperty Source)"
    Write-Host ""
}

# =============================================================================
# VISUAL STUDIO ENVIRONMENT SETUP
# bootstrap-windows.ps1 already verified VS is present and has the C++ workload.
# Here we activate the compiler environment for the current session via
# vcvarsall.bat so that cmake and the MSVC toolchain are on PATH.
# =============================================================================
Write-Host "Setting up Visual Studio compiler environment ($Arch)..."

$VsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $VsWhere)) {
    Write-Host "ERROR: vswhere.exe not found. Is Visual Studio installed?" -ForegroundColor Red
    Write-Host "Install Visual Studio 2019 or later with 'Desktop development with C++' workload."
    exit 1
}

$VsPath    = & $VsWhere -latest `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    -property installationPath 2>$null
$VsVersion = & $VsWhere -latest `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    -property catalog_productDisplayVersion 2>$null

if (-not $VsPath) {
    Write-Host "ERROR: No Visual Studio installation with C++ tools found." -ForegroundColor Red
    exit 1
}

$VcVarsAll = Join-Path $VsPath "VC\Auxiliary\Build\vcvarsall.bat"
if (-not (Test-Path $VcVarsAll)) {
    Write-Host "ERROR: vcvarsall.bat not found. Is 'Desktop development with C++' installed?" -ForegroundColor Red
    exit 1
}

Write-Host "Found Visual Studio $VsVersion"
Write-Host "  at: $VsPath"

$TempFile = [System.IO.Path]::GetTempFileName()
cmd /c "`"$VcVarsAll`" $Arch > nul 2>&1 && set" | Out-File $TempFile -Encoding ASCII
Get-Content $TempFile | ForEach-Object {
    if ($_ -match "^([^=]+)=(.*)$") {
        [System.Environment]::SetEnvironmentVariable($Matches[1], $Matches[2], "Process")
    }
}
Remove-Item $TempFile

Write-Host "Compiler environment ready." -ForegroundColor Green
Write-Host ""

# =============================================================================
# VCPKG SETUP
# Priority: --vcpkg-root arg > US3_VCPKG_ROOT env > source-tree vcpkg > ~/vcpkg
# =============================================================================
$SourceTreeVcpkg = Join-Path $SourceRoot "vcpkg"

if (${vcpkg-root}) {
    $VcpkgRoot = ${vcpkg-root}
    Write-Host "Using vcpkg from --vcpkg-root argument: $VcpkgRoot"
} elseif ($env:US3_VCPKG_ROOT) {
    $VcpkgRoot = $env:US3_VCPKG_ROOT
    Write-Host "Using vcpkg from US3_VCPKG_ROOT environment variable: $VcpkgRoot"
} elseif (Test-Path (Join-Path $SourceTreeVcpkg "bootstrap-vcpkg.bat")) {
    $VcpkgRoot = $SourceTreeVcpkg
    Write-Host "Using vcpkg from source tree: $VcpkgRoot"
} else {
    $VcpkgRoot = Join-Path $HOME "vcpkg"
    Write-Host "Using vcpkg from default location: $VcpkgRoot"
}

Write-Host ""

function Test-VcpkgRoot {
    param([string]$Path)

    if (-not (Test-Path $Path)) { return $false }

    $Required = @(
        "bootstrap-vcpkg.bat",
        "scripts\buildsystems\vcpkg.cmake",
        "ports",
        "triplets"
    )

    foreach ($Item in $Required) {
        if (-not (Test-Path (Join-Path $Path $Item))) {
            return $false
        }
    }

    return $true
}

if (Test-Path $VcpkgRoot) {
    if (Test-VcpkgRoot $VcpkgRoot) {
        Write-Host "Found usable vcpkg root at $VcpkgRoot"
    } else {
        Write-Host "ERROR: $VcpkgRoot exists but is not a usable vcpkg tree." -ForegroundColor Red
        Write-Host "Expected bootstrap-vcpkg.bat, ports, triplets, and scripts\buildsystems\vcpkg.cmake."
        Write-Host "Use --vcpkg-root or US3_VCPKG_ROOT to point to a valid vcpkg path."
        exit 1
    }
} else {
    Write-Host "vcpkg not found at $VcpkgRoot, cloning..."
    git clone https://github.com/microsoft/vcpkg.git $VcpkgRoot
}

if (-not (Test-Path (Join-Path $VcpkgRoot "vcpkg.exe"))) {
    Write-Host "Bootstrapping vcpkg at $VcpkgRoot..."
    Push-Location $VcpkgRoot
    & .\bootstrap-vcpkg.bat -disableMetrics
    Pop-Location
}

# Binary cache: honour US3_VCPKG_CACHE env var, default to $HOME\.vcpkg-cache
$VcpkgCacheDir = if ($env:US3_VCPKG_CACHE) { $env:US3_VCPKG_CACHE } else { Join-Path $HOME ".vcpkg-cache" }
if (-not (Test-Path $VcpkgCacheDir)) { New-Item -ItemType Directory -Path $VcpkgCacheDir | Out-Null }

$env:VCPKG_ROOT           = $VcpkgRoot
$env:VCPKG_BINARY_SOURCES = "clear;files,$VcpkgCacheDir,readwrite"
$env:VCPKG_INSTALLED_DIR  = Join-Path $VcpkgRoot "installed"

if (-not (Test-Path (Join-Path $VcpkgRoot "scripts\buildsystems\vcpkg.cmake"))) {
    Write-Host "ERROR: vcpkg toolchain not found." -ForegroundColor Red
    exit 1
}

Write-Host "vcpkg ready." -ForegroundColor Green
Write-Host ""

# =============================================================================
# CLEAN / REBUILD (tiered)
#
#  --rebuild       Tier 1 : build dir only
#  --clean         Tier 2 : build dir + vcpkg buildtrees + installed/ for triplet
#  --purge-cache   Tier 3 : additive to --clean; also wipes binary cache
# =============================================================================

# Derive the vcpkg triplet for this build.
# Must match what the CMake presets pass as VCPKG_TARGET_TRIPLET.
function Get-VcpkgTriplet {
    if ($Arch -eq "arm64") { return "arm64-windows" }
    else                   { return "x64-windows" }
}

function Remove-BuildDir {
    if (Test-Path $BuildDir) {
        Write-Host "Removing build directory: $BuildDir"
        try {
            Remove-Item -Recurse -Force $BuildDir -ErrorAction Stop
        } catch {
            Write-Warning "Initial removal failed: $($_.Exception.Message)"
            Write-Host "Stopping likely locking processes..."
            Get-Process -ErrorAction SilentlyContinue | Where-Object {
                $_.ProcessName -match '^(assistant|designer|linguist|qtdiag|qtplugininfo|cmake|ctest|us_.*|UltraScan.*)$'
            } | Stop-Process -Force -ErrorAction SilentlyContinue
            Start-Sleep -Seconds 2
            try {
                Remove-Item -Recurse -Force $BuildDir -ErrorAction Stop
            } catch {
                Write-Error "Unable to remove build directory: $BuildDir"
                Write-Error "A file is still locked. Close any running UltraScan/Qt processes and retry."
                exit 1
            }
        }
    } else {
        Write-Host "Build directory does not exist: $BuildDir"
    }
}

function Remove-VcpkgTriplet {
    $Triplet = Get-VcpkgTriplet

    $VcpkgBuildtrees = Join-Path $VcpkgRoot "buildtrees"
    if (Test-Path $VcpkgBuildtrees) {
        Write-Host "Removing vcpkg buildtrees..."
        try   { Remove-Item -Recurse -Force $VcpkgBuildtrees -ErrorAction Stop }
        catch { Write-Warning "Could not fully remove vcpkg buildtrees: $($_.Exception.Message)" }
    }

    # Always wipe the vcpkg bookkeeping directory (status file, .list files,
    # pending updates). Must be unconditional -- stale 'half-installed' entries
    # survive even when the triplet dir was removed by a prior clean, causing
    # vcpkg to fail reading pkgconfig files that no longer exist on next run.
    $VcpkgBookkeeping = Join-Path $VcpkgRoot "installed\vcpkg"
    if (Test-Path $VcpkgBookkeeping) {
        Write-Host "Removing vcpkg installed\vcpkg bookkeeping (will be regenerated)..."
        try   { Remove-Item -Recurse -Force $VcpkgBookkeeping -ErrorAction Stop }
        catch { Write-Warning "Could not fully remove vcpkg bookkeeping: $($_.Exception.Message)" }
    }

    $TripletDir = Join-Path $VcpkgRoot "installed\$Triplet"
    if (Test-Path $TripletDir) {
        Write-Host "Removing vcpkg installed packages for triplet: $Triplet"
        try   { Remove-Item -Recurse -Force $TripletDir -ErrorAction Stop }
        catch { Write-Warning "Could not fully remove triplet dir: $($_.Exception.Message)" }
    } else {
        Write-Host "vcpkg installed\$Triplet does not exist -- nothing to remove"
    }
}

function Remove-BinaryCache {
    if (Test-Path $VcpkgCacheDir) {
        Write-Host "Purging vcpkg binary cache: $VcpkgCacheDir"
        try   { Remove-Item -Recurse -Force $VcpkgCacheDir -ErrorAction Stop }
        catch { Write-Warning "Could not fully purge binary cache: $($_.Exception.Message)" }
        New-Item -ItemType Directory -Path $VcpkgCacheDir -Force | Out-Null
    } else {
        Write-Host "Binary cache does not exist: $VcpkgCacheDir"
    }
}

if (${rebuild} -and -not ${clean}) {
    Write-Host "==========================================" -ForegroundColor Yellow
    Write-Host "Tier 1 rebuild: removing build directory"  -ForegroundColor Yellow
    Write-Host "==========================================" -ForegroundColor Yellow
    Remove-BuildDir
    Write-Host "Rebuild clean complete." -ForegroundColor Green
    Write-Host ""
}

if (${clean}) {
    Write-Host "==========================================" -ForegroundColor Yellow
    Write-Host "Tier 2 clean: build dir + vcpkg installed/" -ForegroundColor Yellow
    Write-Host "==========================================" -ForegroundColor Yellow
    Remove-BuildDir
    Remove-VcpkgTriplet

    if (${purge-cache}) {
        Write-Host "------------------------------------------" -ForegroundColor Yellow
        Write-Host "Tier 3: purging binary cache"              -ForegroundColor Yellow
        Write-Host "------------------------------------------" -ForegroundColor Yellow
        Remove-BinaryCache
    }

    Write-Host "Clean complete." -ForegroundColor Green
    Write-Host ""
}

# =============================================================================
# BUILD SUMMARY
# =============================================================================
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "Ready to build UltraScan3"                 -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "  Platform      : Windows ($Arch)"
Write-Host "  Preset        : $Preset"
Write-Host "  Profile       : ${profile}"
Write-Host "  Qt variant    : $QtLabel"
Write-Host "  Installer     : $(${pkg}.IsPresent)"
Write-Host "  Rebuild       : $(${rebuild}.IsPresent)"
Write-Host "  Clean         : $(${clean}.IsPresent)"
Write-Host "  Purge cache   : $(${purge-cache}.IsPresent)"
Write-Host "  vcpkg root    : $VcpkgRoot"
Write-Host "  Build jobs    : $BuildJobs"
Write-Host ""
Write-Host "Steps:"
Write-Host "  1. Configure CMake with vcpkg toolchain"
Write-Host "  2. Build dependencies (~10 min first time)"
Write-Host "  3. Build UltraScan3 (~5 min)"
if (${pkg}) {
    Write-Host "  4. Stage Qt DLLs and produce NSIS installer"
}
Write-Host ""
if (-not $NonInteractive) {
    if (${rebuild} -and -not ${clean}) {
        Write-Host "Tier 1 rebuild - UltraScan recompiled, vcpkg packages restored from cache" -ForegroundColor Yellow
    } elseif (${clean} -and -not ${purge-cache}) {
        Write-Host "Tier 2 clean - dependencies will be reinstalled (binary cache still warm)" -ForegroundColor Yellow
    } elseif (${clean} -and ${purge-cache}) {
        Write-Host "Tier 3 clean - full recompile from source (binary cache purged)" -ForegroundColor Yellow
    } else {
        Write-Host "Incremental build - only changed files will be rebuilt"
    }
    Write-Host "Grab a coffee if this is your first build!" -ForegroundColor Yellow
    Write-Host ""
}

# =============================================================================
# CONFIGURE AND BUILD
#
# NOTE: Do NOT pass -DCMAKE_TOOLCHAIN_FILE here.
#   toolchain.cmake (set in windows-base preset) handles triplet detection
#   and includes vcpkg via $env:VCPKG_ROOT which is already exported above.
# =============================================================================
Write-Host "Configuring..." -ForegroundColor Cyan
cmake --preset $Preset -DUS3_PROFILE="${profile}"
if ($LASTEXITCODE -ne 0) { Write-Host "ERROR: CMake configuration failed." -ForegroundColor Red; exit $LASTEXITCODE }

Write-Host ""
Write-Host "Building..." -ForegroundColor Cyan
if (${pkg}) {
    Write-Host "Building Windows installer..." -ForegroundColor Cyan
    cmake --build $BuildDir --target package_windows_nsis --parallel $BuildJobs
} else {
    cmake --build $BuildDir --parallel $BuildJobs
}
if ($LASTEXITCODE -ne 0) { Write-Host "ERROR: Build failed." -ForegroundColor Red; exit $LASTEXITCODE }

Write-Host ""
Write-Host "==========================================" -ForegroundColor Green
Write-Host "Build complete!"                           -ForegroundColor Green
Write-Host "==========================================" -ForegroundColor Green
Write-Host ""
if (${pkg}) {
    $InstallerPattern = "build/$Preset/UltraScan3-*-Windows.exe"
    $InstallerFile = Get-ChildItem -Path $InstallerPattern -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($InstallerFile) {
        Write-Host "Installer: $($InstallerFile.FullName)" -ForegroundColor Green
    } else {
        Write-Host "Installer: build/$Preset/UltraScan3-*-Windows.exe" -ForegroundColor Yellow
    }
    Write-Host ""
}
if (-not $NonInteractive) {
    if (-not $DocsBuilt) {
        Write-Host "WARNING: $DocsStatusMessage" -ForegroundColor Yellow
        Write-Host "         Help files (manual.qch/manual.qhc) were not generated." -ForegroundColor Yellow
        Write-Host ""
    }

    Write-Host "Next time you build it will be much faster since dependencies are cached."
    Write-Host ""
    Write-Host "To wipe build dir only:  .\build.bat --rebuild"
    Write-Host "To reinstall vcpkg deps: .\build.bat --clean"
    Write-Host "To recompile everything: .\build.bat --clean --purge-cache"
    if (-not ${pkg}) {
        Write-Host "To build the installer:  .\build.bat --pkg"
    }
    Write-Host ""
}
