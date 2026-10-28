# admin/cmake — CMake Modules

CMake modules and configuration files for the UltraScan3 build system.
Consumed by the root `CMakeLists.txt` and by `build.sh`/`build.ps1`.

## Top-level files

| File | Purpose |
|------|---------|
| `toolchain.cmake` | Resolves vcpkg root, selects the default triplet per platform/arch, sets `VCPKG_INSTALLED_DIR`, and includes `vcpkg.cmake`. Loaded via `CMAKE_TOOLCHAIN_FILE`. |
| `platform-config.cmake` | Compiler flags, RPATH policy, and platform-specific link options common to all targets. |
| `FindQwt.cmake` | Find module for Qwt. Defines `Qwt::Qwt` imported target and `QWT_*` variables. Used when Qwt is not provided by vcpkg. |
| `UsDefinesParser.cmake` | Extracts `US3_VERSION` and `US3_DESCRIPTION` from `us_defines.h` at configure time. |
| `GenerateVersion.cmake` | Generates `us_revision.h` with git commit count, short hash, branch, and dirty flag. Run as a CMake script via `add_custom_target`. |
| `Us3Resources.cmake` | Provides `us3_generate_qrc_from_dir()` for building `.qrc` resource files from a directory glob at configure time. |
| `Us3QrcCheck.cmake` | Validates that all files referenced in a generated `.qrc` actually exist on disk. Run as a CMake script at build time. |

## Subdirectories

| Directory | Purpose |
|-----------|---------|
| `packaging/` | Platform-specific deploy and package CMake modules (macOS PKG, Linux tarball, Windows NSIS). See [`packaging/README.md`](packaging/README.md). |
| `triplets/` | vcpkg overlay triplets for all supported platform/arch combinations. |
| `development-triplets/` | Windows Debug overrides that build both dependency configurations. |

## Windows linking

Keep `CMAKE_JOB_POOL_LINK=us3_link` and `CMAKE_JOB_POOLS=us3_link=1` in
`windows-base`. Executables share `bin/`, and vcpkg copies their DLLs after
each link. Parallel links can race on these files and fail with access errors.
Serializing links keeps build-tree executables runnable locally and in CI.

## Debug builds

Select a Debug preset in CMake or your IDE; no wrapper is required.
Alternatively, use `scripts/build.sh --debug --qt6 APP` on macOS/Linux or
`scripts/build.bat APP --configuration Debug --qt6` on Windows.

Windows Debug presets use `development-triplets/` to build Debug and Release
dependencies, with the matching CRT and Qt plugins. Release presets keep
release-only dependencies. The first Debug build may need to compile Qt.

macOS/Linux Debug presets currently reuse release dependencies to save build
time and disk space. UltraScan itself has debug symbols and no optimization;
stepping through dependency internals requires separate Debug dependencies.
