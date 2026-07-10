$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$emsdkDir = Join-Path $PSScriptRoot "emsdk"
$clionPython = "C:\Program Files\JetBrains\CLion 2025.2.1\bin\lldb\win\x64\bin"

if (Test-Path (Join-Path $clionPython "python.exe")) {
    $env:PATH = "$clionPython;$env:PATH"
}

if (!(Test-Path $emsdkDir)) {
    git clone https://github.com/emscripten-core/emsdk.git $emsdkDir
}

Push-Location $emsdkDir
try {
    .\emsdk.bat install latest
    .\emsdk.bat activate latest
}
finally {
    Pop-Location
}

Write-Host "Emscripten installed in $emsdkDir"
Write-Host "Run tools\build_hello_wasm.ps1 to build the Hello World test."
