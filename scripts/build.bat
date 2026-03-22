@echo off
rem =============================================================================
rem UltraScan3 Windows Build Wrapper
rem =============================================================================
rem Purpose:
rem   Thin Windows wrapper for build.ps1.
rem
rem What it does:
rem   - Locates build.ps1 in the same scripts/ directory
rem   - Accepts either Unix-style "--flag" arguments or PowerShell-style "-flag"
rem   - Normalizes any "--flag" argument to "-flag" before invoking PowerShell
rem   - Forwards all other arguments unchanged
rem
rem Why this exists:
rem   - Lets Windows users and CI use a command style closer to Linux/macOS
rem   - Keeps argument normalization in the wrapper, not in build.ps1
rem   - Preserves build.ps1 as the authoritative Windows build implementation
rem
rem Examples:
rem   scripts\build.bat APP --qt6 --clean --pkg
rem   scripts\build.bat APP -qt6 -clean -pkg
rem   scripts\build.bat --qt6 --clean --pkg
rem
rem Notes:
rem   - If a build profile is provided positionally, it should be APP, TEST, or HPC
rem   - build.ps1 remains responsible for validation and actual build behavior
rem =============================================================================
setlocal EnableExtensions EnableDelayedExpansion

set "SCRIPT_DIR=%~dp0"
set "PS_SCRIPT=%SCRIPT_DIR%build.ps1"

if not exist "%PS_SCRIPT%" (
  echo ERROR: build.ps1 not found at "%PS_SCRIPT%"
  exit /b 1
)

where powershell.exe >nul 2>nul
if errorlevel 1 (
  echo ERROR: powershell.exe not found in PATH.
  exit /b 1
)

set "ARGS="

:parse_args
if "%~1"=="" goto run_build

set "ARG=%~1"

rem If argument starts with --, convert it to single-dash PowerShell style
if "!ARG:~0,2!"=="--" (
  set "ARG=-!ARG:~2!"
)

set "ARGS=!ARGS! "!ARG!""
shift
goto parse_args

:run_build
echo Invoking: %PS_SCRIPT% %ARGS%
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%PS_SCRIPT%" %ARGS%
exit /b %ERRORLEVEL%
