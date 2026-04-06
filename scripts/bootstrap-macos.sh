#!/usr/bin/env bash
# =============================================================================
# bootstrap-macos.sh -- UltraScan3 macOS OS-level prerequisite installer
#
# PURPOSE
# -------
# Installs the Homebrew packages and CLT components that must exist on a
# macOS machine BEFORE build.sh is run.  This script does NOT build
# UltraScan3, does NOT invoke vcpkg, and does NOT duplicate any logic
# that belongs in build.sh.
#
# vcpkg builds almost everything from source, but it still requires a set of
# host tools (cmake, ninja, nasm, pkg-config, python3, etc.) that must be
# present on the system PATH.  On macOS these come from Homebrew.
#
# USAGE
# -----
#   ./scripts/bootstrap-macos.sh [OPTIONS]
#
# OPTIONS
#   --hpc      Also install Open MPI (required for the HPC build profile)
#   --pkg      Also verify macOS packaging tools (required for APP --pkg builds)
#   --dry-run  Print what would be installed without actually installing
#   --help     Show this message and exit
#
# SUPPORTED PLATFORMS
# -------------------
#   macOS 13 Ventura
#   macOS 14 Sonoma
#   macOS 15 Sequoia  <-- primary GitHub Actions runner target (macos-15)
#   Both Apple Silicon (arm64) and Intel (x64)
#
# DESIGN PRINCIPLES
# -----------------
#   - Idempotent: safe to run on a machine where packages are already present
#   - Non-interactive by default in CI (CI=true), prompts locally
#   - Never uses sudo for Homebrew installs (brew refuses to run as root)
#   - Exits clearly with a message when run on a non-macOS platform
# =============================================================================

set -euo pipefail

# =============================================================================
# ARGUMENT PARSING
# =============================================================================
INSTALL_HPC=false
INSTALL_PKG=false
DRY_RUN=false

for arg in "$@"; do
  case "$arg" in
    --hpc)      INSTALL_HPC=true ;;
    --pkg)      INSTALL_PKG=true ;;
    --dry-run)  DRY_RUN=true     ;;
    --help)
      sed -n '/^# USAGE/,/^# ====/p' "$0" | grep -v '^# ====' | sed 's/^# \{0,1\}//'
      exit 0
      ;;
    *)
      echo "ERROR: Unknown option: $arg"
      echo "Run '$0 --help' for usage information."
      exit 1
      ;;
  esac
done

# =============================================================================
# HELPERS
# =============================================================================
log()  { echo "[bootstrap] $*"; }
warn() { echo "[bootstrap] WARNING: $*" >&2; }
die()  { echo "[bootstrap] ERROR: $*" >&2; exit 1; }

# Detect CI mode the same way build.sh does
NON_INTERACTIVE=false
if [ "${CI:-false}" = "true" ]; then
  NON_INTERACTIVE=true
fi

# =============================================================================
# PLATFORM GUARD
# =============================================================================
if [[ "$OSTYPE" != "darwin"* ]]; then
  die "This script is for macOS only. Current OS: ${OSTYPE}
For Linux, use scripts/bootstrap-deps.sh instead."
fi

MACOS_VERSION=$(sw_vers -productVersion 2>/dev/null || echo "unknown")
MACOS_MAJOR=$(echo "$MACOS_VERSION" | cut -d. -f1)
log "Detected: macOS ${MACOS_VERSION}"

if [ "$MACOS_MAJOR" -lt 13 ] 2>/dev/null; then
  die "macOS 13 or later is required. Detected: ${MACOS_VERSION}"
fi

log ""

# =============================================================================
# XCODE COMMAND LINE TOOLS CHECK
# =============================================================================
# The CLT (at /Library/Developer/CommandLineTools) must be present for:
#   - The compiler toolchain (clang, clang++)
#   - Homebrew itself (it checks for CLT on install)
#   - vcpkg bootstrap (which compiles itself)
#
# =============================================================================
log "Checking Xcode Command Line Tools..."

CLT_PATH="/Library/Developer/CommandLineTools"
CLT_MARKER="${CLT_PATH}/usr/bin/clang"

if [ ! -f "$CLT_MARKER" ]; then
  log ""
  log "Xcode Command Line Tools not found at ${CLT_PATH}."
  if [ "$NON_INTERACTIVE" = true ]; then
    # In CI the runner image always has CLT pre-installed.
    # If we somehow got here without them, fail loudly rather than
    # hanging on the interactive xcode-select --install dialog.
    die "Xcode Command Line Tools are required but not installed.
On GitHub Actions runners they are pre-installed. If using a custom runner,
ensure the runner image includes the Command Line Tools package."
  else
    log "About to run: xcode-select --install"
    log "A dialog will appear asking you to install the tools."
    log "Complete the installation, then re-run this script."
    xcode-select --install 2>/dev/null || true
    log ""
    log "After the installation dialog completes, re-run:"
    log "  bash scripts/bootstrap-macos.sh"
    exit 1
  fi
fi

log "Xcode Command Line Tools are present: ${CLT_PATH}"
log ""

# =============================================================================
# XCODE 15/16 DETECTION AND SELECTION
# This belongs in bootstrap because selecting the active developer directory is
# part of preparing the macOS machine to build UltraScan3 successfully.
# =============================================================================
log "Checking Xcode configuration..."

CURRENT_XCODE_PATH="$(xcode-select -p 2>/dev/null || echo "")"
XCODE_DEFAULT_PATH="/Applications/Xcode.app/Contents/Developer"

log "Current Xcode path: ${CURRENT_XCODE_PATH:-<not set>}"
log ""

DESIRED_XCODE_PATH=""
XCODE_CANDIDATES=()

for app in /Applications/Xcode-15*.app /Applications/Xcode-16*.app; do
  [ -d "$app" ] && XCODE_CANDIDATES+=("$app/Contents/Developer")
done
XCODE_CANDIDATES+=("$XCODE_DEFAULT_PATH")

for XCODE_CANDIDATE in "${XCODE_CANDIDATES[@]}"; do
  if [ -d "$XCODE_CANDIDATE" ]; then
    XCODE_APP="$(dirname "$(dirname "$XCODE_CANDIDATE")")"
    XCODE_PLIST="${XCODE_APP}/Contents/Info.plist"
    if [ -f "$XCODE_PLIST" ]; then
      XCODE_VERSION=$(/usr/libexec/PlistBuddy -c "Print :CFBundleShortVersionString" "$XCODE_PLIST" 2>/dev/null || echo "0")
      XCODE_MAJOR="$(echo "$XCODE_VERSION" | cut -d. -f1)"
      if [ "$XCODE_MAJOR" -ge 15 ] 2>/dev/null; then
        DESIRED_XCODE_PATH="$XCODE_CANDIDATE"
        log "Found Xcode ${XCODE_VERSION} at: ${XCODE_CANDIDATE}"
        break
      fi
    fi
  fi
done

if [ -n "$DESIRED_XCODE_PATH" ]; then
  log "Compatible Xcode path: ${DESIRED_XCODE_PATH}"
  if [ "$CURRENT_XCODE_PATH" != "$DESIRED_XCODE_PATH" ]; then
    log "Xcode 15/16 is installed but not active."
    if [ "$NON_INTERACTIVE" = false ]; then
      log "About to switch Xcode to:"
      log "  ${DESIRED_XCODE_PATH}"
      log ""
      read -rp "[bootstrap] Proceed with 'sudo xcode-select --switch ...'? [y/N] " answer
      if [[ "$answer" =~ ^[Yy]$ ]]; then
        log "Switching Xcode..."
        sudo xcode-select --switch "$DESIRED_XCODE_PATH"
        log "Xcode is now set to: $(xcode-select -p)"
      else
        warn "Skipping Xcode switch. Continuing with current Xcode."
      fi
    else
      log "Non-interactive mode: switching Xcode automatically..."
      sudo xcode-select --switch "$DESIRED_XCODE_PATH"
      log "Xcode is now set to: $(xcode-select -p)"
    fi
  else
    log "Compatible Xcode is already active."
  fi
else
  die "Xcode 15 or 16 not found. Checked:
    /Applications/Xcode-15*.app
    /Applications/Xcode-16*.app
    ${XCODE_DEFAULT_PATH}

    Please install Xcode 15 or 16 from the App Store or developer.apple.com."
fi

log ""

# =============================================================================
# macOS PACKAGING TOOLS CHECK
# PKG creation uses Apple packaging tools plus rsync during staging.
# We verify them here when bootstrap is called with --pkg.
# =============================================================================
if [ "${INSTALL_PKG:-false}" = true ]; then
  log "Packaging mode: verifying macOS packaging tools..."

  PKG_VERIFY_TOOLS=(pkgbuild productbuild rsync)
  PKG_MISSING_TOOLS=()

  for tool in "${PKG_VERIFY_TOOLS[@]}"; do
    if ! command -v "$tool" >/dev/null 2>&1; then
      PKG_MISSING_TOOLS+=("$tool")
    fi
  done

  if [ ${#PKG_MISSING_TOOLS[@]} -ne 0 ]; then
    die "Missing required macOS packaging tools: ${PKG_MISSING_TOOLS[*]}
These tools are required for --pkg builds on macOS.
They normally come from Apple developer tools and the base system."
  fi

  log "macOS packaging tools are available."
  log ""
fi

# =============================================================================
# HOMEBREW CHECK / INSTALL
# =============================================================================
log "Checking Homebrew..."

if ! command -v brew &>/dev/null; then
  if [ "$NON_INTERACTIVE" = true ]; then
    # GitHub-hosted macOS runners have Homebrew pre-installed.
    # A missing brew in CI means the PATH is wrong, not that brew is absent.
    # Try the standard install locations before giving up.
    for brew_candidate in \
        /opt/homebrew/bin/brew \
        /usr/local/bin/brew \
        /home/linuxbrew/.linuxbrew/bin/brew; do
      if [ -x "$brew_candidate" ]; then
        eval "$("$brew_candidate" shellenv)"
        log "Found Homebrew at ${brew_candidate}, added to PATH."
        break
      fi
    done
  fi

  if ! command -v brew &>/dev/null; then
    if [ "$NON_INTERACTIVE" = false ]; then
      log "Homebrew not found. Installing..."
      log ""
      log "This will run the official Homebrew installer:"
      log "  https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh"
      log ""
      read -rp "[bootstrap] Proceed with Homebrew installation? [y/N] " answer
      if [[ ! "$answer" =~ ^[Yy]$ ]]; then
        die "Homebrew is required. Install it from https://brew.sh and re-run."
      fi
      /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

      # Add brew to PATH for the remainder of this script
      for brew_candidate in /opt/homebrew/bin/brew /usr/local/bin/brew; do
        if [ -x "$brew_candidate" ]; then
          eval "$("$brew_candidate" shellenv)"
          break
        fi
      done
    else
      die "Homebrew not found and cannot be installed non-interactively.
Ensure Homebrew is installed and on PATH before running build.sh."
    fi
  fi
fi

BREW_PREFIX=$(brew --prefix)
log "Homebrew found: ${BREW_PREFIX}"
log ""

# =============================================================================
# PACKAGE LISTS
# =============================================================================
# Each group is documented with WHY it is needed, not just what it is.
# Keep this as the canonical reference for the macOS Homebrew bootstrap surface.
#
# Note: On macOS, vcpkg uses the Xcode/CLT clang toolchain directly and does
# NOT need a separate compiler formula.  The packages here are tools that vcpkg
# requires to be on PATH during port builds, or that CMake/build.sh need.

# --- Core build tools -------------------------------------------------------
# cmake: project build system (requires >= 3.21; brew ships current release)
# ninja: the generator used by every CMake preset (see CMakePresets.json)
# git: vcpkg clones itself and its ports from GitHub; also needed for checkout
# pkg-config: used by vcpkg during port configure steps; the linux-base preset
#             sets PKG_CONFIG_PATH explicitly, macOS equivalent relies on PATH
BREW_PKGS_TOOLCHAIN=(
  cmake
  ninja
  git
  pkg-config
  autoconf
  autoconf-archive
  automake
  libtool
)

# --- Assembly ---------------------------------------------------------------
# nasm: mainly relevant for x86/x86_64 assembly-heavy paths.
#       On Apple Silicon it is usually unnecessary, so skip it to avoid
#       prompting developers for a tool they may never need.
MACOS_ARCH="$(uname -m)"
INSTALL_NASM=false

if [ "$MACOS_ARCH" = "x86_64" ]; then
  INSTALL_NASM=true
else
  log "Apple Silicon detected; skipping nasm."
fi

# --- Python (Sphinx documentation) ------------------------------------------
# python3: Sphinx is installed via pip from doc/manual/source/requirements.txt.
#          build.sh handles the actual pip install; we ensure python3 is present.
#          Homebrew python3 is preferred over the system stub in /usr/bin.
#          Note: macOS ships a /usr/bin/python3 stub that prompts for CLT install
#          rather than running Python -- the Homebrew version is the real one.
BREW_PKGS_PYTHON=(
  python3
)

# --- Archive utilities ------------------------------------------------------
# curl: vcpkg uses curl to download port source archives.
#       macOS ships its own curl but it may lack required TLS features on
#       older OS versions; the Homebrew version is more reliable.
# Note: zip/unzip/tar are already present in macOS base system.
BREW_PKGS_ARCHIVE=(
  curl
)

# --- MPI (optional, --hpc flag) ---------------------------------------------
# Required only for the HPC build profile (mpicxx must be on PATH for build.sh).
# Open MPI is the standard choice; its Homebrew formula requires the CLT to be
# present at /Library/Developer/CommandLineTools (already verified above).
BREW_PKGS_HPC=(
  open-mpi
)

# =============================================================================
# BUILD PACKAGE LIST
# =============================================================================
ALL_BREW_PKGS=()

for pkg in "${BREW_PKGS_TOOLCHAIN[@]}"; do
  ALL_BREW_PKGS+=("$pkg")
done

if [ "$INSTALL_NASM" = true ]; then
  ALL_BREW_PKGS+=("nasm")
fi

for pkg in "${BREW_PKGS_PYTHON[@]}"; do
  ALL_BREW_PKGS+=("$pkg")
done

for pkg in "${BREW_PKGS_ARCHIVE[@]}"; do
  ALL_BREW_PKGS+=("$pkg")
done

if [ "$INSTALL_HPC" = true ]; then
  ALL_BREW_PKGS+=("${BREW_PKGS_HPC[@]}")
  log "HPC mode: Open MPI will be included."
fi

# =============================================================================
# DRY-RUN MODE
# =============================================================================
if [ "$DRY_RUN" = true ]; then
  log "Dry-run mode -- nothing will be installed."
  log ""
  log "Packages that would be installed (if missing):"
  for pkg in "${ALL_BREW_PKGS[@]}"; do
    echo "  $pkg"
  done
  log ""
  log "Command that would run:"
  log "  brew install ${ALL_BREW_PKGS[*]}"
  exit 0
fi

# =============================================================================
# FILTER: ONLY INSTALL WHAT IS MISSING
# =============================================================================
# Developer-friendly behavior:
# - If a usable tool is already on PATH, do not force a Brew install just
#   because Brew does not "own" it.
# - Only fall back to Brew installation when the tool is actually missing.
# - For formulas whose executable name differs from the formula name, map them
#   explicitly (e.g. open-mpi -> mpicxx).
#
log "Checking which packages are already installed or otherwise available..."

PKGS_TO_INSTALL=()
tool_for_formula() {
  case "$1" in
    cmake)             echo "cmake" ;;
    ninja)             echo "ninja" ;;
    git)               echo "git" ;;
    pkg-config)        echo "pkg-config" ;;
    autoconf)          echo "autoconf" ;;
    automake)          echo "automake" ;;
    libtool)           echo "glibtoolize" ;;
    nasm)              echo "nasm" ;;
    python3)           echo "python3" ;;
    curl)              echo "curl" ;;
    open-mpi)          echo "mpicxx" ;;
    autoconf-archive)  echo "" ;;
    *)                 echo "$1" ;;
  esac
}

tool_is_usable() {
  local formula="$1"
  local tool
  tool="$(tool_for_formula "$formula")"

  case "$formula" in
    autoconf-archive)
      brew list --formula autoconf-archive >/dev/null 2>&1
      ;;

    python3)
      command -v python3 >/dev/null 2>&1 || return 1
      python3 - <<'PY' >/dev/null 2>&1
import sys
print(sys.version)
PY
      ;;

    *)
      [ -n "$tool" ] && command -v "$tool" >/dev/null 2>&1
      ;;
  esac
}

for pkg in "${ALL_BREW_PKGS[@]}"; do
  if tool_is_usable "$pkg"; then
    tool="$(tool_for_formula "$pkg")"
    if [ -n "$tool" ]; then
      log "Found usable tool for $pkg: $(command -v "$tool" 2>/dev/null || echo "<unknown>")"
    else
      log "Found usable Homebrew formula for $pkg."
    fi
    continue
  fi

  # Tool not usable on PATH; check whether Brew already has the formula.
  if brew list --formula "$pkg" >/dev/null 2>&1; then
    if [ "$NON_INTERACTIVE" = true ]; then
      die "Homebrew formula $pkg is installed, but required tool is not usable on PATH. Expected Homebrew bin under ${BREW_PREFIX}/bin."
    fi
    log "Homebrew formula $pkg is installed, but its tool is not currently usable on PATH."
    log "Will not reinstall automatically; verify your PATH includes: ${BREW_PREFIX}/bin"
    continue
  fi

  PKGS_TO_INSTALL+=("$pkg")
done

if [ ${#PKGS_TO_INSTALL[@]} -eq 0 ]; then
  log "No Homebrew packages need installation."
  log "Proceeding to final verification..."
  log ""
else
  log ""
  log "Packages to install (${#PKGS_TO_INSTALL[@]}):"
  for pkg in "${PKGS_TO_INSTALL[@]}"; do
    echo "  $pkg"
  done
  log ""

  # =============================================================================
  # CONFIRMATION (interactive mode only)
  # =============================================================================
  if [ "$NON_INTERACTIVE" = false ]; then
    read -rp "[bootstrap] Proceed with 'brew install'? [y/N] " answer
    if [[ ! "$answer" =~ ^[Yy]$ ]]; then
      log "Aborted. No packages were installed."
      exit 0
    fi
  fi

  # =============================================================================
  # INSTALL
  # =============================================================================
  if [ "$NON_INTERACTIVE" = true ]; then
    HOMEBREW_NO_AUTO_UPDATE=1 \
    HOMEBREW_NO_INSTALL_CLEANUP=1 \
    brew install "${PKGS_TO_INSTALL[@]}"
  else
    brew install "${PKGS_TO_INSTALL[@]}"
  fi
fi

# =============================================================================
# POST-INSTALL VERIFICATION
# =============================================================================
log ""
log "Verifying key tools..."

VERIFY_TOOLS=(cmake ninja git pkg-config autoconf automake glibtoolize python3 xcodebuild xcrun)

if [ "$INSTALL_NASM" = true ]; then
  VERIFY_TOOLS+=(nasm)
fi

if [ "$INSTALL_HPC" = true ]; then
  VERIFY_TOOLS+=(mpicxx)
fi

if [ "$INSTALL_PKG" = true ]; then
  VERIFY_TOOLS+=(pkgbuild productbuild rsync)
fi

MISSING_AFTER=()
for tool in "${VERIFY_TOOLS[@]}"; do
  if ! command -v "$tool" &>/dev/null; then
    MISSING_AFTER+=("$tool")
  fi
done

if ! brew list --formula autoconf-archive >/dev/null 2>&1; then
  if [ "$NON_INTERACTIVE" = true ]; then
    die "Required formula autoconf-archive is not installed after Homebrew install."
  fi
  warn "Required formula autoconf-archive is not installed."
fi

if [ ${#MISSING_AFTER[@]} -ne 0 ]; then
  warn "The following tools are still not on PATH after installation:"
  for tool in "${MISSING_AFTER[@]}"; do
    warn "  $tool  (expected at ${BREW_PREFIX}/bin/${tool})"
  done
  warn ""
  warn "Ensure ${BREW_PREFIX}/bin is on your PATH. Add this to your shell profile:"
  warn "  eval \"\$(${BREW_PREFIX}/bin/brew shellenv)\""

  if [ "$NON_INTERACTIVE" = true ]; then
    die "Required tools are still missing after Homebrew installation."
  fi

  warn "Build may fail. Please investigate before running build.sh."
else
  log "All key tools verified on PATH."
fi

# =============================================================================
# DONE
# =============================================================================
log ""
log "Bootstrap complete."
log ""
log "Next steps:"
log "  1. Run 'bash scripts/build.sh --help' to see build options."
log "  2. First build: 'bash scripts/build.sh --qt6'  (or --qt5-qwt616)"
log "     (vcpkg will build all library dependencies; allow 30-45 minutes)"
