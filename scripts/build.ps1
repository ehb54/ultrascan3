param(
    [string]$ConfigurePreset = "windows-release-qt6",
    [string]$BuildPreset     = "windows-release-qt6"
)
$ErrorActionPreference = "Stop"
Write-Host "=== UltraScan Windows Build ===" -ForegroundColor Cyan

Set-Location (Resolve-Path "$PSScriptRoot\..")

if (!(Test-Path ".\vcpkg")) {
    Write-Host "ERROR: vcpkg folder not found." -ForegroundColor Red; exit 1
}
if (!(Test-Path ".\vcpkg\vcpkg.exe")) {
    Write-Host "Bootstrapping vcpkg..."
    & .\vcpkg\bootstrap-vcpkg.bat
}

Write-Host "Configuring with preset: $ConfigurePreset"
cmake --preset $ConfigurePreset

Write-Host "Building with preset: $BuildPreset"
cmake --build --preset $BuildPreset --parallel

Write-Host "Build complete." -ForegroundColor Green