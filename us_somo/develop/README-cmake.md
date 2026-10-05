# Building SOMO with CMake

The CMake build compiles the same files as qmake: it reads the source lists
from `libus_somo.pro` and each program's `.pro` when it configures. It builds
`libus_somo` and the programs `us3_somo`, `us_admin`, `us3_config` and (not on
Windows) `us_saxs_cmds_t`.

## Against the Qt and Qwt of the qmake build

The presets take Qt 5 and the qmake-built Qwt from `QTDIR` and `QWTDIR`, which
`qt5env` sets.

From a terminal, in `us_somo/develop` after sourcing `qt5env`:

    cmake --preset qt5
    cmake --build --preset qt5 -j 8

In the MSYS2 MINGW64 shell use the `qt5-msys2` preset. CMake there must be a
Windows build of CMake, such as the zip from cmake.org.

Everything lands in `build/<preset>/`: the programs in `bin/`, the library in
`lib/` (in `bin/` on Windows). After editing sources, only the build step is
needed; it reruns the configure step itself when a `.pro` file changes.

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

Both need the root CMake and vcpkg files (`admin/cmake`,
`buildsys/vcpkg`), which main has:

- standalone: `docker/` holds a Linux build environment with vcpkg; see its
  README;
- from the repository root: the root presets with `-DUS3_BUILD_SOMO=ON`
  build SOMO along with the rest of UltraScan.
