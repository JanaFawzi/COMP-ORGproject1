$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$node = Join-Path $PSScriptRoot "emsdk\node\22.16.0_64bit\bin\node.exe"

Push-Location $root
try {
    $backendJs = Join-Path $root "web-build\zx16_backend.js"
    $backendWasm = Join-Path $root "web-build\zx16_backend.wasm"
    $webBuild = Join-Path $root "web\build"

    if (!(Test-Path $webBuild)) {
        New-Item -ItemType Directory -Force -Path $webBuild | Out-Null
    }

    if (Test-Path $backendJs) {
        Copy-Item -LiteralPath $backendJs -Destination $webBuild -Force

        if (Test-Path $backendWasm) {
            Copy-Item -LiteralPath $backendWasm -Destination $webBuild -Force
        }
    }
    else {
        Write-Host "Backend WASM not found yet. Run tools\build_hello_wasm.ps1 before the full page test."
    }

    if (Test-Path $node) {
        & $node web\build_frontend.js
    }
    else {
        node web\build_frontend.js
    }

    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }
}
finally {
    Pop-Location
}
