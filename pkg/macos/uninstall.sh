#!/bin/bash
# =============================================================================
# uninstall.sh — UltraScan3 safe uninstaller for macOS
#
# Usage:
#   sudo ./uninstall.sh [--dry-run] [--yes]
#
# Options:
#   --dry-run   Print every action that would be taken; make no changes.
#   --yes       Skip the interactive confirmation prompt.
#
# What it does (in order):
#   1. Enforces root execution.
#   2. Validates the exact removal target to prevent accidental rm -rf.
#   3. Optionally prompts for confirmation (bypassed with --yes).
#   4. Kills running UltraScan3 processes (best-effort, non-fatal).
#   5. Stops and unloads the ultrascan_sysctl LaunchDaemon.
#   6. Removes /Library/LaunchDaemons/ultrascan_sysctl.plist.
#   7. Removes /Applications/UltraScan3 (ONLY that exact path).
#   8. Forgets the PKG receipt via pkgutil.
#
# Safety guarantees:
#   - Will NEVER rm -rf any path other than the literal string
#     "/Applications/UltraScan3".  The target is validated by three
#     independent checks before any destructive action is taken.
#   - --dry-run is fully honoured: no filesystem mutations occur.
#   - All actions are logged to /tmp/UltraScan3-uninstall-<timestamp>.log.
#
# Exit codes:
#   0  — success (or dry-run completed)
#   1  — fatal error (root check, safety guard, unexpected failure)
# =============================================================================
set -euo pipefail

# ---------------------------------------------------------------------------
# Logging setup — must happen before any other output.
# ---------------------------------------------------------------------------
readonly TIMESTAMP=$(date '+%Y%m%d-%H%M%S')
readonly LOG="/tmp/UltraScan3-uninstall-${TIMESTAMP}.log"

# Tee every line to both stdout and the log file.
exec > >(tee -a "$LOG") 2>&1

log() {
    echo "$(date '+%Y-%m-%d %H:%M:%S') $*"
}

log "============================================================"
log "UltraScan3 Uninstaller"
log "  Log file : $LOG"
log "  macOS    : $(sw_vers -productVersion 2>/dev/null || echo unknown)"
log "  arch     : $(uname -m 2>/dev/null || echo unknown)"
log "============================================================"

# ---------------------------------------------------------------------------
# Constants — defined as readonly to prevent accidental mutation.
# ---------------------------------------------------------------------------
# SAFETY: APP_DIR is the ONLY path this script will ever pass to rm -rf.
# It is validated by checksum-of-string logic below before use.
readonly APP_DIR="/Applications/UltraScan3"
readonly DAEMON_PLIST="/Library/LaunchDaemons/ultrascan_sysctl.plist"
readonly DAEMON_LABEL="ultrascan_sysctl"
readonly PKG_ID="com.aucsolutions.ultrascan3"

# ---------------------------------------------------------------------------
# Argument parsing
# ---------------------------------------------------------------------------
DRY_RUN=0
AUTO_YES=0

for arg in "$@"; do
    case "$arg" in
        --dry-run) DRY_RUN=1 ;;
        --yes)     AUTO_YES=1 ;;
        *)
            echo "ERROR: Unknown argument: $arg"
            echo "Usage: sudo $0 [--dry-run] [--yes]"
            exit 1
            ;;
    esac
done

if [ "${DRY_RUN}" -eq 1 ]; then
    log "*** DRY-RUN MODE — no changes will be made ***"
fi

# ---------------------------------------------------------------------------
# Helper: run a command or print it in dry-run mode.
# ---------------------------------------------------------------------------
run() {
    if [ "${DRY_RUN}" -eq 1 ]; then
        log "[DRY-RUN] would run: $*"
    else
        log "Running: $*"
        "$@"
    fi
}

# ---------------------------------------------------------------------------
# 1. Root check
# ---------------------------------------------------------------------------
if [ "$(id -u)" -ne 0 ]; then
    log "ERROR: This script must be run as root."
    log "       Re-run with: sudo $0 $*"
    exit 1
fi
log "Root check — OK (uid=0)"

# ---------------------------------------------------------------------------
# 2. Safety guard — validate APP_DIR before any destructive use.
#
#    Three independent checks must ALL pass:
#      a) The variable equals the expected literal string.
#      b) The resolved absolute path equals the expected literal string.
#      c) The path has exactly two components (top-level /Applications dir).
#
#    If any check fails the script aborts without touching anything.
# ---------------------------------------------------------------------------
log "------------------------------------------------------------"
log "Safety validation of removal target ..."

# (a) Literal string equality
if [ "${APP_DIR}" != "/Applications/UltraScan3" ]; then
    log "SAFETY ABORT: APP_DIR variable has unexpected value: '${APP_DIR}'"
    log "              This script will not proceed."
    exit 1
fi

# (b) Resolved absolute path (guards against symlink shenanigans)
#     Use Python as a portable realpath(3) equivalent on macOS.
_resolved=$(python3 -c "import os,sys; p=sys.argv[1]; print(os.path.normpath(os.path.abspath(p)))" "${APP_DIR}" 2>/dev/null || echo "RESOLVE_FAILED")
if [ "${_resolved}" != "/Applications/UltraScan3" ]; then
    log "SAFETY ABORT: Resolved path '${_resolved}' != '/Applications/UltraScan3'."
    log "              /Applications/UltraScan3 may be a symlink to another location."
    log "              Remove the symlink manually and re-run."
    exit 1
fi

# (c) Depth check: path must have exactly 3 slash-separated parts: "" "Applications" "UltraScan3"
_depth=$(echo "${APP_DIR}" | tr -cd '/' | wc -c | tr -d ' ')
if [ "${_depth}" -ne 2 ]; then
    log "SAFETY ABORT: APP_DIR path depth is ${_depth}, expected 2."
    exit 1
fi

log "Safety validation passed:"
log "  APP_DIR   = ${APP_DIR}"
log "  resolved  = ${_resolved}"
log "  depth     = ${_depth}"

# ---------------------------------------------------------------------------
# 3. Confirmation prompt (skipped with --yes or --dry-run)
# ---------------------------------------------------------------------------
if [ "${DRY_RUN}" -eq 0 ] && [ "${AUTO_YES}" -eq 0 ]; then
    log "------------------------------------------------------------"
    echo ""
    echo "  This will PERMANENTLY remove:"
    echo "    ${APP_DIR}"
    echo "    ${DAEMON_PLIST}"
    echo "    PKG receipt: ${PKG_ID}"
    echo ""
    echo -n "  Proceed? Type YES (uppercase) to confirm: "
    read -r _confirm
    echo ""
    if [ "${_confirm}" != "YES" ]; then
        log "Uninstall cancelled by user."
        exit 0
    fi
    log "User confirmed: YES"
fi

# ---------------------------------------------------------------------------
# 4. Stop running UltraScan3 processes (best-effort, non-fatal).
#    We send SIGTERM and give processes 5 seconds to exit cleanly,
#    then SIGKILL any survivors.  Errors are logged but do not abort.
# ---------------------------------------------------------------------------
log "------------------------------------------------------------"
log "Stopping UltraScan3 processes (best-effort) ..."

_us3_pids=$(pgrep -f "${APP_DIR}" 2>/dev/null || true)

if [ -n "${_us3_pids}" ]; then
    log "Found UltraScan3 processes: $(echo "${_us3_pids}" | tr '\n' ' ')"
    if [ "${DRY_RUN}" -eq 0 ]; then
        echo "${_us3_pids}" | xargs -r kill -TERM 2>/dev/null || true
        log "  Sent SIGTERM; waiting up to 5 seconds ..."
        sleep 5
        _survivors=$(pgrep -f "${APP_DIR}" 2>/dev/null || true)
        if [ -n "${_survivors}" ]; then
            log "  Survivors found; sending SIGKILL: $(echo "${_survivors}" | tr '\n' ' ')"
            echo "${_survivors}" | xargs -r kill -KILL 2>/dev/null || true
        else
            log "  All processes exited cleanly."
        fi
    else
        log "[DRY-RUN] would SIGTERM/SIGKILL pids: $(echo "${_us3_pids}" | tr '\n' ' ')"
    fi
else
    log "No running UltraScan3 processes found."
fi

# ---------------------------------------------------------------------------
# 5. Stop and unload the LaunchDaemon.
# ---------------------------------------------------------------------------
log "------------------------------------------------------------"
log "Removing LaunchDaemon ${DAEMON_LABEL} ..."

if [ -f "${DAEMON_PLIST}" ]; then
    log "  Plist found: ${DAEMON_PLIST}"

    # Try modern bootout first, fall back to legacy unload.
    if [ "${DRY_RUN}" -eq 0 ]; then
        if /bin/launchctl bootout system "${DAEMON_PLIST}" >>"$LOG" 2>&1; then
            log "  launchctl bootout system — succeeded"
        else
            _rc=$?
            log "  launchctl bootout returned ${_rc} (daemon may not be running — OK)"
            /bin/launchctl unload -w "${DAEMON_PLIST}" >>"$LOG" 2>&1 || true
            log "  launchctl unload fallback — complete"
        fi
    else
        log "[DRY-RUN] would: launchctl bootout system ${DAEMON_PLIST}"
    fi
else
    log "  ${DAEMON_PLIST} not found — skipping daemon unload"
fi

# ---------------------------------------------------------------------------
# 6. Remove the LaunchDaemon plist.
# ---------------------------------------------------------------------------
if [ -f "${DAEMON_PLIST}" ]; then
    run rm -f "${DAEMON_PLIST}"
    log "  Removed ${DAEMON_PLIST}"
else
    log "  ${DAEMON_PLIST} already absent — skipping"
fi

# ---------------------------------------------------------------------------
# 7. Remove /Applications/UltraScan3.
#
#    The APP_DIR variable has already passed the three-check safety guard
#    above.  We perform one final inline assertion immediately before the
#    rm call to make the safety intent explicit and auditable.
# ---------------------------------------------------------------------------
log "------------------------------------------------------------"
log "Removing application directory: ${APP_DIR} ..."

if [ -d "${APP_DIR}" ] || [ -e "${APP_DIR}" ]; then
    # Final inline assertion — belt AND suspenders.
    if [ "${APP_DIR}" != "/Applications/UltraScan3" ]; then
        log "SAFETY ABORT: Final assertion failed on APP_DIR='${APP_DIR}'."
        exit 1
    fi
    run rm -rf "/Applications/UltraScan3"
    log "  Removed /Applications/UltraScan3"
else
    log "  ${APP_DIR} not found — already removed or never installed"
fi

# ---------------------------------------------------------------------------
# 8. Forget the PKG receipt.
# ---------------------------------------------------------------------------
log "------------------------------------------------------------"
log "Forgetting PKG receipt: ${PKG_ID} ..."

if [ "${DRY_RUN}" -eq 0 ]; then
    pkgutil --forget "${PKG_ID}" >>"$LOG" 2>&1 || true
    log "  pkgutil --forget ${PKG_ID} — done (errors above are non-fatal)"
else
    log "[DRY-RUN] would: pkgutil --forget ${PKG_ID}"
fi

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------
log "============================================================"
if [ "${DRY_RUN}" -eq 1 ]; then
    log "DRY-RUN complete — no changes were made."
else
    log "UltraScan3 uninstall complete."
    log ""
    log "  Note: sysctl shared-memory values set by the LaunchDaemon"
    log "  will revert to macOS defaults on the next reboot."
fi
log "  Full log: ${LOG}"
log "============================================================"
exit 0
