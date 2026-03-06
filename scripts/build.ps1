#Requires -Version 5.1
<#
.SYNOPSIS
    UltraScan3 Windows build script.

.DESCRIPTION
    Configures and builds UltraScan3 using CMake presets and vcpkg.
    Uses the same arguments as build.sh on macOS/Linux.

.PARAMETER qt6
    Build with Qt6 + Qwt6.3.0 [default]

.PARAMETER qt5_qwt616
    Build with Qt5 + Qwt6.1.6

.PARAMETER qt5_qwt630
    Build with Qt5 + Qwt6.3.0

.PARAMETER arch
    Target architecture: x64 (default, auto-detect) or arm64

.PARAMETER profile
    Build profile: APP (default), TEST, HPC

.PARAMETER vcpkg_root
    Path to vcpkg installation. Priority: --vcpkg-root > US3_VCPKG_ROOT env > source-tree vcpkg > ~/vcpkg

.PARAMETER clean
    Remove build artifacts before building

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
    .\build.bat --clean --arch arm64
    Clean then build Qt6, ARM64, APP profile

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
#>

param(
    [switch]${clean},
    [switch]${pkg},
    [switch]${qt6},
    [switch]${qt5-qwt616},
    [switch]${qt5-qwt630},
    [string]${arch}       = "",
    [string]${profile}    = "APP",
    [string]${vcpkg-root} = "",
    [switch]${help}
)

$DocsBuilt = $true
$DocsStatusMessage = ""

# =============================================================================
# HELP
# =============================================================================
if (${help}) {
    Write-Host "Usage: build.bat [OPTIONS] [PROFILE]"
    Write-Host ""
    Write-Host "OPTIONS:"
    Write-Host "  --clean                  Clean the active build directory and vcpkg buildtrees before building"
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
    Write-Host "  build.bat --clean                          # Clean then build Qt6, APP"
    Write-Host "  build.bat --clean --qt5-qwt616             # Clean Qt5 + Qwt6.1.6"
    Write-Host "  build.bat --clean --arch arm64 TEST        # Clean ARM64 Qt6 TEST build"
    Write-Host "  build.bat --vcpkg-root C:\dev\vcpkg        # Use specific vcpkg"
    Write-Host "  build.bat --vcpkg-root .\vcpkg             # Use source-tree vcpkg"
    Write-Host "  build.bat --pkg                            # Build + produce NSIS installer"
    Write-Host ""
    Write-Host "ENVIRONMENT VARIABLES:"
    Write-Host "  US3_BUILD_JOBS           Override number of parallel build jobs"
    Write-Host "  US3_VCPKG_ROOT           Override vcpkg location (see priority above)"
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
Write-Host "Selected Qt version    : $QtSuffix"
Write-Host "Architecture           : $Arch"
Write-Host "Preset                 : $Preset"
if (${clean}) { Write-Host "Clean build requested" -ForegroundColor Yellow }
if (${pkg})   { Write-Host "Installer build requested" -ForegroundColor Cyan }
Write-Host ""

# =============================================================================
# BUILD PARALLELISM
# =============================================================================
$Cores = if ($env:NUMBER_OF_PROCESSORS) { [int]$env:NUMBER_OF_PROCESSORS } else { 4 }
if ($env:US3_BUILD_JOBS) {
    $BuildJobs = [int]$env:US3_BUILD_JOBS
} else {
    $BuildJobs = [Math]::Max(1, [Math]::Floor($Cores * 0.9))
}

Write-Host "Detected $Cores cores; using $BuildJobs parallel build jobs."
Write-Host ""

$env:VCPKG_MAX_CONCURRENCY = "$BuildJobs"

# =============================================================================
# NSIS PATH SELF-HEAL
# If makensis is not on PATH, check the two standard install locations and
# add whichever one exists to the session PATH so the tool-check below passes.
# =============================================================================
if (-not (Get-Command makensis -ErrorAction SilentlyContinue)) {
    $NsisPaths = @(
        "C:\Program Files (x86)\NSIS",
        "C:\Program Files\NSIS"
    )
    foreach ($NsisDir in $NsisPaths) {
        if (Test-Path (Join-Path $NsisDir "makensis.exe")) {
            Write-Host "Found NSIS at $NsisDir -- adding to PATH for this session." -ForegroundColor Yellow
            $env:PATH = "$NsisDir;$env:PATH"
            break
        }
    }
}

# =============================================================================
# CHECK REQUIRED TOOLS
# =============================================================================
Write-Host "Checking required tools..."

# Base tools required for all builds
$RequiredTools = @("cmake", "git", "ninja")

# Packaging also requires NSIS
if (${pkg}) {
    $RequiredTools += "makensis"
}

# Map tool name -> install instructions for helpful error messages.
# sphinx-build is listed separately below as a warning (not a hard failure).
$WingetMap = @{
    cmake    = "Kitware.CMake"
    git      = "Git.Git"
    ninja    = "Ninja-build.Ninja"
    makensis = "NSIS.NSIS"
}

$MissingTools = @()
foreach ($Tool in $RequiredTools) {
    if (-not (Get-Command $Tool -ErrorAction SilentlyContinue)) {
        $MissingTools += $Tool
    }
}

if ($MissingTools.Count -gt 0) {
    Write-Host "ERROR: Missing required tools: $($MissingTools -join ', ')" -ForegroundColor Red
    Write-Host ""
    Write-Host "Install the missing tools with:" -ForegroundColor Yellow
    Write-Host ""
    foreach ($Tool in $MissingTools) {
        if ($WingetMap.ContainsKey($Tool)) {
            Write-Host "  winget install $($WingetMap[$Tool])"
        } else {
            Write-Host "  Install tool manually: $Tool"
        }
    }
    Write-Host ""
    exit 1
}

# Sphinx is required for documentation (manual.qch / manual.qhc).
# It is not a hard build failure -- the build succeeds without it, but the
# help system will not work in the installed application.
if (-not (Get-Command sphinx-build -ErrorAction SilentlyContinue)) {
    $DocsBuilt = $false
    $DocsStatusMessage = "Documentation not built: sphinx-build not found."

    Write-Host "  Install Python 3.x first, for example:" -ForegroundColor Yellow
    Write-Host "    winget install -e --id Python.Python.3.13" -ForegroundColor Yellow
    Write-Host "  Then install the documentation dependencies:" -ForegroundColor Yellow
    Write-Host "    py -m pip install -r doc/manual/source/requirements.txt" -ForegroundColor Yellow
    Write-Host ""
}

Write-Host "All required tools are available." -ForegroundColor Green
Write-Host ""

# =============================================================================
# VISUAL STUDIO ENVIRONMENT SETUP
# =============================================================================
Write-Host "Setting up Visual Studio compiler environment ($Arch)..."

$VsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $VsWhere)) {
    Write-Host "ERROR: vswhere.exe not found. Is Visual Studio installed?" -ForegroundColor Red
    Write-Host "Install Visual Studio 2019 or later with 'Desktop development with C++' workload."
    exit 1
}

$VsPath    = & $VsWhere -latest -property installationPath 2>$null
$VsVersion = & $VsWhere -latest -property catalog_productDisplayVersion 2>$null

if (-not $VsPath) {
    Write-Host "ERROR: No Visual Studio installation found." -ForegroundColor Red
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

if ((Test-Path $VcpkgRoot) -and (-not (Test-Path (Join-Path $VcpkgRoot ".git")))) {
    Write-Host "ERROR: $VcpkgRoot exists but is not a vcpkg git clone." -ForegroundColor Red
    Write-Host "Use --vcpkg-root or US3_VCPKG_ROOT to point to a valid vcpkg path."
    exit 1
}

if (-not (Test-Path (Join-Path $VcpkgRoot ".git"))) {
    Write-Host "vcpkg not found at $VcpkgRoot, cloning..."
    git clone https://github.com/microsoft/vcpkg.git $VcpkgRoot
}

if (-not (Test-Path (Join-Path $VcpkgRoot "vcpkg.exe"))) {
    Write-Host "Bootstrapping vcpkg at $VcpkgRoot..."
    Push-Location $VcpkgRoot
    & .\bootstrap-vcpkg.bat
    Pop-Location
}

$env:VCPKG_ROOT           = $VcpkgRoot
$env:VCPKG_BINARY_SOURCES = "clear;files,$HOME\.vcpkg-cache,readwrite"
$env:VCPKG_INSTALLED_DIR  = Join-Path $VcpkgRoot "installed"

$VcpkgCacheDir = Join-Path $HOME ".vcpkg-cache"
if (-not (Test-Path $VcpkgCacheDir)) { New-Item -ItemType Directory -Path $VcpkgCacheDir | Out-Null }

if (-not (Test-Path (Join-Path $VcpkgRoot "scripts\buildsystems\vcpkg.cmake"))) {
    Write-Host "ERROR: vcpkg toolchain not found." -ForegroundColor Red
    exit 1
}

Write-Host "vcpkg ready." -ForegroundColor Green
Write-Host ""

# =============================================================================
# CLEAN (if requested)
# =============================================================================
if ($clean) {
    Write-Host "==========================================" -ForegroundColor Yellow
    Write-Host "Cleaning build artifacts..."               -ForegroundColor Yellow
    Write-Host "==========================================" -ForegroundColor Yellow

    # Remove only the active build tree for this invocation
    if (Test-Path $BuildDir) {
        Write-Host "Removing active build directory: $BuildDir"

        try {
            Remove-Item -Recurse -Force $BuildDir -ErrorAction Stop
        }
        catch {
            Write-Warning "Initial removal of build directory failed:"
            Write-Warning $_.Exception.Message

            Write-Host "Stopping likely locking processes..."
            Get-Process -ErrorAction SilentlyContinue | Where-Object {
                $_.ProcessName -match '^(assistant|designer|linguist|qtdiag|qtplugininfo|cmake|ctest|us_.*|UltraScan.*)$'
            } | Stop-Process -Force -ErrorAction SilentlyContinue

            Start-Sleep -Seconds 2

            try {
                Remove-Item -Recurse -Force $BuildDir -ErrorAction Stop
            }
            catch {
                Write-Error "Unable to remove build directory: $BuildDir"
                Write-Error "A file is still locked. Close Explorer windows or any running UltraScan/Qt executables and retry."
                exit 1
            }
        }
    }
    else {
        Write-Host "Active build directory does not exist: $BuildDir"
    }

    $VcpkgBuildtrees = Join-Path $VcpkgRoot "buildtrees"
    if (Test-Path $VcpkgBuildtrees) {
        Write-Host "Removing vcpkg build trees..."

        try {
            Remove-Item -Recurse -Force $VcpkgBuildtrees -ErrorAction Stop
        }
        catch {
            Write-Warning "Could not fully remove vcpkg buildtrees:"
            Write-Warning $_.Exception.Message
        }
    }

    # Uncomment for a full dependency clean (much slower):
    # $VcpkgInstalled = Join-Path $VcpkgRoot "installed"
    # if (Test-Path $VcpkgInstalled) {
    #     Write-Host "Removing vcpkg installed packages..."
    #     Remove-Item -Recurse -Force $VcpkgInstalled
    # }
    #
    # $VcpkgPackages = Join-Path $VcpkgRoot "packages"
    # if (Test-Path $VcpkgPackages) {
    #     Write-Host "Removing vcpkg package cache..."
    #     Remove-Item -Recurse -Force $VcpkgPackages
    # }

    Write-Host "Clean complete." -ForegroundColor Green
    Write-Host ""
}

# =============================================================================
# BUILD SUMMARY
# =============================================================================
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "Ready to build UltraScan3"                 -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "  Preset        : $Preset"
Write-Host "  Profile       : ${profile}"
Write-Host "  Qt version    : $QtSuffix"
Write-Host "  Architecture  : $Arch"
Write-Host "  Clean build   : $(${clean}.IsPresent)"
Write-Host "  Installer     : $(${pkg}.IsPresent)"
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
    Write-Host "To rebuild from scratch: .\build.bat --clean"
    if (-not ${pkg}) {
        Write-Host "To build the installer:  .\build.bat --pkg"
    }
    Write-Host ""
}
