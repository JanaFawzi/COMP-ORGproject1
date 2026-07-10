$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$node = Join-Path $PSScriptRoot "emsdk\node\22.16.0_64bit\bin\node.exe"

Push-Location $root
try {
    & powershell -ExecutionPolicy Bypass -File tools\build_hello_wasm.ps1

    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }

    if (!(Test-Path $node)) {
        Write-Error "Emscripten Node was not found at $node."
    }

    & $node web\build_test_backend.js

    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }

    & $node web-build\ts\test_backend.js

    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }
}
finally {
    Pop-Location
}
