@echo off
:: =============================================================================
:: UltraScan3 Windows Bootstrap Launcher
:: =============================================================================
:: Installs the tools and components required before building UltraScan3:
::   - Git, CMake, Ninja, NASM, Python 3, NSIS (via winget)
::   - Visual Studio with 'Desktop development with C++' workload (verified,
::     not auto-installed -- see bootstrap-windows.ps1 for remediation steps)
::
:: Run this once on a new machine before running build.bat.
:: It is safe to run again -- already-installed tools are skipped.
::
:: USAGE:
::     scripts\bootstrap.bat           Install all prerequisites
::     scripts\bootstrap.bat --dry-run Show what would be installed
::
:: To edit bootstrap behavior, see: scripts\bootstrap-windows.ps1
:: =============================================================================
powershell.exe -ExecutionPolicy Bypass -File "%~dp0bootstrap-windows.ps1" %*
