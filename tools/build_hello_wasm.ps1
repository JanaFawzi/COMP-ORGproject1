$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$emsdkDir = Join-Path $PSScriptRoot "emsdk"
$emsdkEnv = Join-Path $emsdkDir "emsdk_env.bat"
$emcmake = Join-Path $emsdkDir "upstream\emscripten\emcmake.exe"
$clionPython = "C:\Program Files\JetBrains\CLion 2025.2.1\bin\lldb\win\x64\bin"
$clionCmake = "C:\Program Files\JetBrains\CLion 2025.2.1\bin\cmake\win\x64\bin"
$clionNinja = "C:\Program Files\JetBrains\CLion 2025.2.1\bin\ninja\win\x64"
$cmake = Join-Path $clionCmake "cmake.exe"
$ninja = Join-Path $clionNinja "ninja.exe"

if (Test-Path (Join-Path $clionPython "python.exe")) {
    $env:PATH = "$clionPython;$env:PATH"
}

if (Test-Path (Join-Path $clionCmake "cmake.exe")) {
    $env:PATH = "$clionCmake;$env:PATH"
}

if (!(Test-Path $emsdkEnv)) {
    Write-Error "Emscripten is not installed. Run tools\install_emscripten.ps1 first."
}

if (!(Test-Path $emcmake)) {
    Write-Error "emcmake was not found. Run tools\install_emscripten.ps1 again."
}

if (!(Test-Path $cmake)) {
    Write-Error "CMake was not found at $cmake."
}

if (!(Test-Path $ninja)) {
    Write-Error "Ninja was not found at $ninja."
}

cmd /c "call `"$emsdkEnv`" && `"$emcmake`" `"$cmake`" -S web -B web-build -G Ninja -DCMAKE_MAKE_PROGRAM=`"$ninja`" && `"$cmake`" --build web-build"

if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

Write-Host "Built web-build\hello_wasm.html and web-build\zx16_backend.js"
