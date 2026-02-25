@echo off
:: =============================================================================
:: UltraScan3 Windows Build Launcher
:: =============================================================================
:: This file is a thin passthrough to build.ps1 which contains all the build
:: logic. You can run it from any terminal (CMD, PowerShell, or Windows
:: Terminal) without needing to set up an execution policy manually.
::
:: All arguments are forwarded directly to build.ps1. Run with --help to see
:: all available options:
::
::     scripts\build.bat --help
::
:: EXAMPLES:
::     scripts\build.bat                            Qt6, auto-detect arch, APP
::     scripts\build.bat --arch arm64               Qt6, ARM64, APP
::     scripts\build.bat --qt5-qwt616               Qt5 + Qwt6.1.6, APP
::     scripts\build.bat --clean                    Clean then build
::     scripts\build.bat --clean --arch arm64 TEST  Clean ARM64 Qt6 TEST build
::     scripts\build.bat --vcpkg-root C:\dev\vcpkg  Use specific vcpkg
::
:: To edit build behavior, see: scripts\build.ps1
:: =============================================================================
powershell.exe -ExecutionPolicy Bypass -File "%~dp0build.ps1" %*

