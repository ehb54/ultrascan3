# pkg/macos — macOS PKG Installer Assets

Static assets consumed by `admin/cmake/packaging/macos/MacOsPkg.cmake`
and `admin/cmake/packaging/macos/MacDeploy.cmake`
when building the macOS PKG installer.

## Contents

| Path | Purpose |
|------|---------|
| `distribution.xml.in` | productbuild distribution definition; `@US3_VERSION_STRING@` is substituted by CMake at configure time |
| `component.plist` | pkgbuild component plist; declares relocatability and upgrade behaviour for `us.app` and `Assistant.app` |
| `preinstall` | Installer script run as root before payload is laid down; checks for XQuartz and unloads any existing LaunchDaemon |
| `postinstall` | Installer script run as root after payload; sets sysctl shared-memory values and bootstraps the LaunchDaemon |
| `ultrascan_sysctl.plist` | LaunchDaemon plist installed to `/Library/LaunchDaemons/`; applies sysctl values at boot |
| `uninstall.sh` | Staged into `/Applications/UltraScan3/`; provides a supported removal path for end users |
| `launcher/UltraScan3.app` | Dock launcher app bundle template; staged at the root of the install by `MacDeploy.cmake` so users can drag it to the Dock. Contains a shell script that opens `bin/us.app`. |
| `resources/Welcome.txt.in` | Installer welcome screen text; version-stamped by CMake |
| `resources/background.png` | Installer background image shown by the macOS Installer UI |

## Relationship to CMake packaging

Build logic lives entirely in `admin/cmake/packaging/macos/`.
This directory contains only static source assets — no CMake code.

`MacOsPkg.cmake` references these files as `${CMAKE_SOURCE_DIR}/pkg/macos/...`
and stages them into the build tree under `${CMAKE_BINARY_DIR}/pkg/macos/`
before invoking `pkgbuild` and `productbuild`.

`MacDeploy.cmake` additionally copies `launcher/UltraScan3.app` into the
root of the staging directory (`_stage/UltraScan3/`) so the launcher is
included in the final PKG payload at `/Applications/UltraScan3/UltraScan3.app`.

Do not move or rename files here without updating `MacOsPkg.cmake` and `MacDeploy.cmake`.