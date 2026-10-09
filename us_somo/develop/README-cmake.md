# Building SOMO with CMake

The CMake build compiles the same files as qmake: it reads the source lists
from `libus_somo.pro` and each program's `.pro` when it configures. It builds
`libus_somo` and the programs `us3_somo`, `us_admin` (CMake target
`us3_admin`), `us3_config` and (not on Windows, as under qmake)
`us_saxs_cmds_t`. The MPI and CUDA tools are not in the CMake build yet; qmake
still builds them.

On Windows SOMO builds with MinGW (MSYS2), as qmake builds it for the
packages. MSVC does not compile it yet (ehb54/ultrascan-tickets#1131): the
root build skips SOMO under MSVC with a warning, and a SOMO-only configure
stops.

## SOMO only

`us_somo/develop` configures on its own and builds only SOMO; it neither
builds nor needs any other part of UltraScan. It works on somo-dev too, and
even with nothing but the `us_somo/develop` directory. Without the rest of the
repository, or without git, the version headers in `include/` are kept if they
are already there (somo-dev commits them) and are otherwise written with
version "unknown" and revision 0. The macOS programs then also lose their
icons, which live in `us_somo/etc`.

## Against the Qt and Qwt of the qmake build

The presets take Qt 5 and the qmake-built Qwt from `QTDIR` and `QWTDIR`, which
`qt5env` sets.

From a terminal, in `us_somo/develop` after sourcing `qt5env`:

    cmake --preset qt5
    cmake --build --preset qt5 -j 8

In the MSYS2 MINGW64 shell use the `qt5-msys2` preset. CMake there must be a
Windows build of CMake, such as the zip from cmake.org.

The compiler warnings are qmake's (`-Wall -Wextra`, with the macOS
packaging's `-Wno-deprecated*`). To see each compile and link command, add
`--verbose` to the build command; `build/<preset>/compile_commands.json` lists
the exact command for each file.

Everything lands in `build/<preset>/`: the programs in `bin/` (on macOS as
`.app` bundles, as qmake builds them, so `bin/us3_somo.app/Contents/MacOS/us3_somo`),
the library in `lib/` (in `bin/` on Windows). After editing sources, only the
build step is needed; it reruns the configure step itself when a `.pro` file
changes.

The programs run from the build tree. SOMO takes the directory above a
program's `bin/` as its system directory: always on Linux and Windows, and on
macOS when `ULTRASCAN` is not set. So the configure step lays out
`build/<preset>/` as SOMO expects:
- `somo/` and `etc/` linked to `us_somo`'s (copied on Windows where links
  cannot be made);
- the helper programs from `us_somo/add_to_bin` (GRPY, iftci) next to the
  programs;
- on Linux, `bin64` linked to `bin`, as in an installation.

To run the programs, source `qt5env` first. On macOS they find the Qwt
framework through its `DYLD_FRAMEWORK_PATH`; on Windows put `$QTDIR/bin` and
`$QWTDIR/lib` on `PATH`.

## VS Code and Qt Creator

Both read `CMakePresets.json`. Open `us_somo/develop` as the project folder
(or, in VS Code, set `cmake.sourceDirectory` to it). The presets need `QTDIR`
and `QWTDIR`: start the editor from a shell that has sourced `qt5env` (on
Windows, the MINGW64 shell), or name the paths in a `CMakeUserPresets.json`
next to `CMakePresets.json`, which git ignores:

    {
      "version": 3,
      "configurePresets": [
        {
          "name": "my-qt5",
          "inherits": "qt5",
          "environment": {
            "QTDIR": "/Users/me/src/qt-5.15.19",
            "QWTDIR": "/Users/me/src/qt-5.15.19-qwt-6.3.0"
          }
        }
      ]
    }

On Windows inherit from `qt5-msys2` instead, with paths such as
`C:/msys64/home/me/src/qt-5.15.14`.

macOS removes `DYLD_*` variables from the environment of programs it starts
for the editor, so to run or debug SOMO from VS Code set
`DYLD_FRAMEWORK_PATH` to `<QTDIR>/lib:<QWTDIR>/lib` in the launch
configuration.

## With vcpkg

These need the whole repository, as on main: they use the root's vcpkg setup
(`admin/cmake`, the root `vcpkg.json`, `buildsys/vcpkg`).

- SOMO only, on macOS and Linux, from `us_somo/develop`:

      cmake --preset vcpkg-qt6
      cmake --build --preset vcpkg-qt6

  `vcpkg-qt5` builds against Qt 5, and the `-debug` presets build Debug.
  The presets set vcpkg up as the root presets do: the root manifest with its
  `qt5-app` / `qt6-app` feature, and `admin/cmake`'s toolchain wrapper and
  overlay triplets. So they install the same packages as a root build, and
  share its binary cache. The packages go in
  `build/<preset>/vcpkg_installed`. vcpkg is found as for the root presets:
  `$VCPKG_ROOT`, or `~/vcpkg`.
- From the repository root: the root presets with `-DUS3_BUILD_SOMO=ON`
  build SOMO along with the rest of UltraScan (not under MSVC, see above).
- `docker/` holds a Linux build environment for both; see its README.
