<#
.SYNOPSIS
    Build the host-test binary (game logic on 6502 + py65 model).
.DESCRIPTION
    Transforms src/main.c (see transform.py), compiles game + test
    driver with cc65 for 6502, links a 32 KB test image. Ship sources
    are never modified; the transform works on a disposable copy.
#>
$ErrorActionPreference = "Stop"
$Here = $PSScriptRoot
$ProjectRoot = Split-Path -Parent (Split-Path -Parent $Here)
$Out = "$Here\out"
if (-not (Test-Path $Out)) { New-Item -ItemType Directory -Path $Out | Out-Null }

$CC65 = "C:\cc65\bin\cc65.exe"
$CA65 = "C:\cc65\bin\ca65.exe"
$LD65 = "C:\cc65\bin\ld65.exe"
$LIB = "C:\cc65\lib\none.lib"

# Run a native toolchain exe without letting stderr warnings become
# terminating errors (EAP=Stop + native stderr = death in PS 5.1;
# same pattern as scripts/build.ps1 Invoke-Toolchain).
function Invoke-Toolchain {
    param([string]$Exe, [Parameter(ValueFromRemainingArguments = $true)]$ToolArgs)
    $prevEap = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    & $Exe @ToolArgs
    $ErrorActionPreference = $prevEap
}

Write-Host "Transforming..." -ForegroundColor Green
python "$Here\transform.py"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Compiling game.c (6502)..." -ForegroundColor Green
Invoke-Toolchain $CC65 @("-t", "none", "--cpu", "6502", "-I$ProjectRoot\src", "-O", "-o", "$Out\game.s", "$Out\game.c")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Invoke-Toolchain $CA65 @("--cpu", "6502", "-o", "$Out\game.o", "$Out\game.s")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Compiling m8.c (6502, flat: FARCODE/FARRODATA only exist on hardware)..." -ForegroundColor Green
Invoke-Toolchain $CC65 @("-t", "none", "--cpu", "6502", "-I$ProjectRoot\src", "-O", "-o", "$Out\m8.s", "$Out\m8.c")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Invoke-Toolchain $CA65 @("--cpu", "6502", "-o", "$Out\m8.o", "$Out\m8.s")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Compiling test_main.c..." -ForegroundColor Green
Invoke-Toolchain $CC65 @("-t", "none", "--cpu", "6502", "-O", "-o", "$Out\test_main.s", "$Here\test_main.c")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Invoke-Toolchain $CA65 @("--cpu", "6502", "-o", "$Out\test_main.o", "$Out\test_main.s")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Assembling test_crt.s..." -ForegroundColor Green
Invoke-Toolchain $CA65 @("--cpu", "6502", "-o", "$Out\test_crt.o", "$Here\test_crt.s")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Linking test image..." -ForegroundColor Green
Invoke-Toolchain $LD65 @("-C", "$Here\host.cfg", "-o", "$Out\test.bin", "-m", "$Out\test.map", "-Ln", "$Out\labels.txt", "$Out\game.o", "$Out\m8.o", "$Out\test_main.o", "$Out\test_crt.o", $LIB)
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Write-Host "Built $Out\test.bin" -ForegroundColor Cyan
