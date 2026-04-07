# admin/cmake/packaging — Platform Packaging Modules

CMake modules that stage and package UltraScan3 for distribution.
Included by the root `CMakeLists.txt` based on the target platform.

Each platform subdirectory follows the same pattern:
- **Deploy** module — stages the build tree into a self-contained directory tree
- **Pkg** module — wraps the staged tree into a distributable package

Static installer assets (scripts, plists, XML templates, resources) live in
`pkg/<platform>/` at the repo root, not here. These modules reference those
assets as `${CMAKE_SOURCE_DIR}/pkg/<platform>/...`.

## macos/

| File | Purpose |
|------|---------|
| `MacDeploy.cmake` | Stages `_stage/UltraScan3/` with `.app` bundles, Qt frameworks, plugins, `etc/`, `somo/`, and the `UltraScan3.app` Dock launcher. Runs `macdeployqt` and fixes rpaths. |
| `MacOsPkg.cmake` | Builds the final `.pkg` installer via `pkgbuild` + `productbuild` from the staged tree. |
| `MacHpcDeploy.cmake` | HPC variant of `MacDeploy.cmake` — headless, no GUI, no Qt frameworks. |
| `MacHpcPkg.cmake` | HPC variant of `MacOsPkg.cmake` — produces a tarball instead of a PKG. |
| `MacOsDeployQtAssistant.cmake` | Deploys Qt Assistant.app into the staging tree with correct rpaths and bundle ID patching. |
| `StageAssistantDev.cmake` | Builds a local `Assistant.app` from the dynamic triplet for development use without a full deploy. |

## linux/

| File | Purpose |
|------|---------|
| `LinuxDeploy.cmake` | Stages the Linux install tree with shared libraries, Qt plugins, and `patchelf` rpath fixes. |
| `LinuxPkg.cmake` | Produces the `.tar.gz` distribution archive from the staged tree. |
| `LinuxHpcDeploy.cmake` | HPC variant — headless staging, MPI binaries only. |
| `LinuxHpcPkg.cmake` | HPC variant — produces a tarball from the HPC staged tree. |

## windows/

| File | Purpose |
|------|---------|
| `WinDeploy.cmake` | Stages the Windows install tree using `windeployqt`. |
| `WinPkg.cmake` | Produces the NSIS `.exe` installer from the staged tree. |
| `WindowsDeployQtAssistant.cmake` | Deploys Qt Assistant into the Windows staging tree. |


