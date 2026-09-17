<#>
.SYNOPSIS
    Build script for Star Merchants SNES using cc65 toolchain + PVSnesLib
.DESCRIPTION
    Compiles C/asm sources, links with PVSnesLib, outputs .sfc ROM
.NOTES
    Requires: cc65 suite (cl65, ca65, ld65) in PATH or CC65_HOME
    Requires: PVSNESLIB_HOME environment variable set
#>

param(
    [string]$Target = "LoROM_SlowROM",
    [switch]$Clean,
    [switch]$Run,
    [string]$Emu = "mesen-s",
    [string]$RomName = "starmerchants"
)

$ErrorActionPreference = "Stop"

# --- Configuration ---
$CC65_HOME = if ($env:CC65_HOME) { $env:CC65_HOME } else { "C:\cc65" }
$PVSNESLIB_HOME = if ($env:PVSNESLIB_HOME) { $env:PVSNESLIB_HOME } else { "C:\dev\snes\tools\pvsneslib\pvsneslib" }

if (-not (Test-Path "$CC65_HOME\bin\cl65.exe")) {
    Write-Error "cl65.exe not found at $CC65_HOME\bin\cl65.exe"
    exit 1
}
if (-not (Test-Path $PVSNESLIB_HOME)) {
    Write-Error "PVSNESLIB_HOME not found at $PVSNESLIB_HOME"
    exit 1
}

$CL65 = "$CC65_HOME\bin\cl65.exe"
$CA65 = "$CC65_HOME\bin\ca65.exe"
$LD65 = "$CC65_HOME\bin\ld65.exe"

$SRC_DIR = "src"
$BUILD_DIR = "build"
$ROM_FILE = "$BUILD_DIR\$RomName.sfc"
$MAP_FILE = "$BUILD_DIR\$RomName.map"
$SYM_FILE = "$BUILD_DIR\$RomName.sym"

$PVS_INCLUDE = "$PVSNESLIB_HOME\include"
$PVS_LIB_DIR = "$PVSNESLIB_HOME\lib\$Target"
$DEVKIT_INCLUDE = "C:\pvsneslib\devkitsnes\include"

# PVSnesLib object files (pre-built with 816-tcc)
$PVS_OBJECTS = @(
    "$PVS_LIB_DIR\crt0_snes.obj",
    "$PVS_LIB_DIR\libc.obj",
    "$PVS_LIB_DIR\libm.obj",
    "$PVS_LIB_DIR\libtcc.obj"
)

# Verify PVSnesLib objects exist
foreach ($obj in $PVS_OBJECTS) {
    if (-not (Test-Path $obj)) {
        Write-Error "Missing PVSnesLib object: $obj"
        exit 1
    }
}

# --- Clean ---
if ($Clean) {
    Write-Host "Cleaning $BUILD_DIR..." -ForegroundColor Yellow
    if (Test-Path $BUILD_DIR) { Remove-Item -Recurse -Force $BUILD_DIR }
    exit 0
}

# --- Setup build dir ---
if (-not (Test-Path $BUILD_DIR)) { New-Item -ItemType Directory -Path $BUILD_DIR | Out-Null }

# --- Find source files ---
$CFiles = Get-ChildItem -Path $SRC_DIR -Recurse -Filter "*.c" | Select-Object -ExpandProperty FullName
$ASMFiles = Get-ChildItem -Path $SRC_DIR -Recurse -Filter "*.asm" | Select-Object -ExpandProperty FullName
$SFiles = Get-ChildItem -Path $SRC_DIR -Recurse -Filter "*.s" | Select-Object -ExpandProperty FullName

if (-not $CFiles -and -not $ASMFiles -and -not $SFiles) {
    Write-Warning "No source files found in $SRC_DIR"
    # Create a minimal hello world for testing
    $testC = @"
#include <snes.h>
#include <stdio.h>

int main() {
    consoleInit();
    consoleSetTextVramBGAddress(0x6800);
    consoleSetTextVramAddress(0x3000);
    consoleSetTextOffset(0x0100);
    consoleInitText(0, 16*2, &font8x8);
    puts("STAR MERCHANTS");
    puts("Press START to continue...");
    while(1) {
        scanPads();
        if (padsCurrent(0) & KEY_START) break;
        WaitForVBlank();
    }
    return 0;
}
"@
    $testPath = "$SRC_DIR\main.c"
    if (-not (Test-Path $SRC_DIR)) { New-Item -ItemType Directory -Path $SRC_DIR | Out-Null }
    Set-Content -Path $testPath -Value $testC
    $CFiles = @($testPath)
    Write-Host "Created minimal main.c for testing" -ForegroundColor Cyan
}

# --- Compile C files to .o ---
$ObjFiles = @()
foreach ($c in $CFiles) {
    $obj = "$BUILD_DIR\$([IO.Path]::GetFileNameWithoutExtension($c)).o"
    Write-Host "Compiling $c..." -ForegroundColor Green
    & $CL65 -t none --cpu 65816 -I"$PVS_INCLUDE" -I"$DEVKIT_INCLUDE" -I"$SRC_DIR" -O -c -o $obj $c
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    $ObjFiles += $obj
}

# --- Assemble ASM files ---
foreach ($asm in $ASMFiles) {
    $obj = "$BUILD_DIR\$([IO.Path]::GetFileNameWithoutExtension($asm)).o"
    Write-Host "Assembling $asm..." -ForegroundColor Green
    & $CA65 --cpu 65816 -I"$PVS_INCLUDE" -I"$DEVKIT_INCLUDE" -I"$SRC_DIR" -o $obj $asm
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    $ObjFiles += $obj
}

# --- Link ---
Write-Host "Linking $ROM_FILE..." -ForegroundColor Green
$linkArgs = @(
    "-C", "$CC65_HOME\cfg\snestarget.cfg",
    "-o", $ROM_FILE,
    "-m", $MAP_FILE,
    "--dbgfile", $SYM_FILE,
    "--obj", $PVS_OBJECTS,
    "--obj", $ObjFiles,
    "-u", "__STARTUP__",
    "-u", "__IRQ_VECTOR__",
    "-u", "__NMI_VECTOR__",
    "-u", "__RESET_VECTOR__"
)

& $LD65 @linkArgs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

# --- Verify ROM ---
$romSize = (Get-Item $ROM_FILE).Length
Write-Host "ROM built: $ROM_FILE ($([math]::Round($romSize/1024,1)) KB)" -ForegroundColor Cyan

# --- Run in emulator ---
if ($Run) {
    $emuPath = @(
        "C:\Program Files\Mesen-S\Mesen-S.exe",
        "C:\Program Files (x86)\Mesen-S\Mesen-S.exe",
        "$env:LOCALAPPDATA\Mesen-S\Mesen-S.exe",
        "C:\Program Files\bsnes\bsnes.exe",
        "C:\Program Files\snes9x\snes9x.exe"
    ) | Where-Object { Test-Path $_ } | Select-Object -First 1

    if ($emuPath) {
        Write-Host "Launching $emuPath..." -ForegroundColor Cyan
        & $emuPath $ROM_FILE
    } else {
        Write-Warning "No emulator found. Install Mesen-S, bsnes, or snes9x."
    }
}