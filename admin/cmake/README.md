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

## Windows: linking is deliberately serialized

The `windows-base` preset in `CMakePresets.json` sets:

```json
"CMAKE_JOB_POOLS": "us3_link=1",
"CMAKE_JOB_POOL_LINK": "us3_link"
```

so only one target links at a time. **This is a correctness fix, not a leftover. Do not remove it to speed up Windows builds.**

vcpkg's `VCPKG_APPLOCAL_DEPS` defaults to on for Windows, which appends a `vcpkg z-applocal` pass to every executable link to copy dependent DLLs into the output `bin/`. All ~72 executables in `programs/` share a single `bin/`, so parallel links run concurrent `z-applocal` passes over the same DLLs. Windows has no unlink-while-open, so they race and the build fails like this:

```
[758/827] Linking CXX executable bin\us_2dsa.exe
FAILED: [code=1] bin/us_2dsa.exe
  ... link.exe ... && ... vcpkg.exe z-applocal --target-binary=.../bin/us_2dsa.exe
open_for_read(".../bin\icudt78.dll"): permission denied
```

The link itself succeeds — the post-link DLL copy is what fails. Which target dies varies from run to run, so it reads like a linker or toolchain problem rather than a concurrency bug. First observed in CI run `32604671061`; fixed by commit `e1312c9e1`.

This is not a CI-runner limitation. It affects local Windows builds too, and is *more* likely on machines with many cores.

The alternative — `VCPKG_APPLOCAL_DEPS=OFF` plus one deployment pass at the end — was considered and rejected. It would leave `build/<preset>/bin/*.exe` without their Qt/ICU DLLs, so they could no longer be launched directly from the build tree, which is part of the normal development workflow here. Packaging would be unaffected, since `packaging/windows/WinDeploy.cmake` runs its own full `windeployqt`.

`CMakePresets.json` is strict JSON and rejects unrecognized keys, so this rationale cannot be recorded next to the setting itself.

## Windows: debug builds use the release CRT

The Windows overlay triplets set `VCPKG_BUILD_TYPE release`, so dependencies exist in release form only. MSVC makes the C runtime part of the ABI — `/MDd` raises `_ITERATOR_DEBUG_LEVEL` and stamps a `detect_mismatch` directive into every object — so a `/MDd` build of UltraScan3 cannot link against `/MD` Qt (hard `LNK2038`, not a warning).

`windows-base` therefore pins `CMAKE_MSVC_RUNTIME_LIBRARY` to `MultiThreadedDLL`. Debug presets still produce unoptimized code with full symbols; they just use the release CRT. Removing that pin, or reverting the triplets to debug+release, are the two coherent alternatives — but the second invalidates the prebuilt Windows toolchain archive pinned in `buildsys/toolchain.lock.json`, which contains release binaries only.

