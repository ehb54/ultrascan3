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

