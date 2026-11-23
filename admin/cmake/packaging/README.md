# admin/cmake/packaging — Platform Packaging Modules

CMake modules that stage and package UltraScan3 for distribution.
Included by the root `CMakeLists.txt` based on the target platform.

Each platform subdirectory follows the same pattern:
- **Deploy** module — stages the build tree into a self-contained directory tree
- **Pkg** module — wraps the staged tree into a distributable package

Static installer assets (scripts, plists, XML templates, resources) live in
`pkg/<platform>/` at the repo root, not here. These modules reference those
assets as `${CMAKE_SOURCE_DIR}/pkg/<platform>/...`.

## APP package inputs

`programs/CMakeLists.txt` registers the executable targets selected by its
platform and MPI rules. `AppPackage.cmake` uses those targets to generate
`app-package-<configuration>.cmake`, containing exact target output paths
(including renamed executables and macOS bundles). The same inventory feeds
`us3_app_package_inputs`, which depends on all selected programs and documentation.

The APP package targets can therefore be invoked directly after configuration:

```sh
cmake --build <build-dir> --config Release --target package_linux_tarball
cmake --build <build-dir> --config Release --target package_windows_nsis
cmake --build <build-dir> --config Release --target package_macos_pkg
```

Use the target for the configured platform. APP packaging requires documentation
to be enabled, Sphinx, and qhelpgenerator; developer binary builds may omit these.
`StageAppPrograms.cmake` rejects missing/empty executable outputs and copies only
the generated inventory. `ValidateAppHelp.cmake` rejects missing/empty staged
manuals, Assistant, or SQLite help components. Each deployment starts with a clean
stage. A standalone deploy-script invocation must supply `APP_PROGRAM_MANIFEST`
pointing to the generated manifest for its build configuration.

Optional SOMO outputs remain separately staged without new build dependencies.
Their legacy input-directory scans are restricted to SOMO executable names, so
that directory cannot reintroduce stale core programs. HPC executable staging is unchanged.

Run the packaging fixtures with:

```sh
python3 -m unittest discover -s test/packaging -p 'test_*.py'
```

The clean-build tests use tiny compiled programs and stub Qt deployment tools.
Linux tests invoke the real tarball target; Windows/macOS staging is inspected
through fixture archives, not native NSIS/PKG installers. macOS bundle staging
requires a macOS host; the multi-configuration test requires Ninja.

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
| `LinuxPkg.cmake` | Produces the `.tar.xz` distribution archive from the staged tree. |
| `LinuxHpcDeploy.cmake` | HPC variant — headless staging, MPI binaries only. |
| `LinuxHpcPkg.cmake` | HPC variant — produces a tarball from the HPC staged tree. |

## windows/

| File | Purpose |
|------|---------|
| `WinDeploy.cmake` | Stages the Windows install tree using `windeployqt`. |
| `WinPkg.cmake` | Produces the NSIS `.exe` installer from the staged tree. |
| `WindowsDeployQtAssistant.cmake` | Deploys Qt Assistant into the Windows staging tree. |



## Exact package output

Every APP and HPC package target clears `package-path.txt` before packaging and
writes the final absolute filename after verifying a nonempty output. The build
wrappers read that receipt using `scripts/report-package.sh` or
`scripts/report-package.ps1` and publish `package_path` through `GITHUB_OUTPUT`.
The upload action stages only that file at the artifact ZIP root. Missing or
invalid receipts and missing or empty packages fail explicitly; unrelated old
packages never participate in selection.
