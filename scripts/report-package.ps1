param([Parameter(Mandatory = $true)][string]$ReceiptPath)
$ErrorActionPreference = 'Stop'
# Read the successful package target's receipt; never search build directories.
$Package = [IO.File]::ReadAllText((Resolve-Path -LiteralPath $ReceiptPath)).TrimEnd([char[]]"`r`n")
if ($Package -match '[\r\n]' -or -not [IO.Path]::IsPathRooted($Package)) {
    throw 'Package receipt must contain one absolute path'
}
$Item = Get-Item -LiteralPath $Package -ErrorAction Stop
if ($Item.PSIsContainer -or $Item.Length -eq 0) {
    throw "Declared package is empty or not a file: $Package"
}
Write-Host "Package: $Package"
if ($env:GITHUB_OUTPUT) {
    [IO.File]::AppendAllText($env:GITHUB_OUTPUT, "package_path=$Package`n", [Text.UTF8Encoding]::new($false))
}
