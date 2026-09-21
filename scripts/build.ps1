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

# Use script directory as base for relative paths
$ScriptDir = $PSScriptRoot
$ProjectRoot = Split-Path -Parent $ScriptDir

# --- Configuration ---
$CC65_HOME = if ($env:CC65_HOME) { $env:CC65_HOME } else { "C:\cc65" }
$PVSNESLIB_HOME = if ($env:PVSNESLIB_HOME) { 
    # Support both: C:\dev\snes\tools\pvsneslib or C:\dev\snes\tools\pvsneslib\pvsneslib
    if (Test-Path "$env:PVSNESLIB_HOME\pvsneslib") { "$env:PVSNESLIB_HOME\pvsneslib" } 
    else { $env:PVSNESLIB_HOME } 
} else { "C:\dev\snes\tools\pvsneslib\pvsneslib" }

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

# Exact port of Mesen-S BaseCartridge::GetHeaderScore (Core/BaseCartridge.cpp).
# The emulator scores header candidates; only a score >= 0 gets loaded.
function Get-SnesHeaderScore([byte[]]$rom, [int]$addr) {
    $romSize = $rom.Length
    if ($romSize -lt ($addr + 0x7FFF)) { return -1 }
    $b = $addr + 0x7FB0
    $mapMode  = [int]$rom[$b + 37]
    $romType  = [int]$rom[$b + 38]
    $sizeByte = [int]$rom[$b + 39]
    $sramByte = [int]$rom[$b + 40]
    $comp     = [int]$rom[$b + 44] + ([int]$rom[$b + 45] * 256)
    $chk      = [int]$rom[$b + 46] + ([int]$rom[$b + 47] * 256)

    $score = 0
    $mode = $mapMode -band 0xEF        # MapMode & ~0x10
    if ((($mode -eq 0x20) -or ($mode -eq 0x22)) -and ($addr -lt 0x8000)) { $score++ }
    elseif ((($mode -eq 0x21) -or ($mode -eq 0x25)) -and ($addr -ge 0x8000)) { $score++ }
    if ($romType -lt 0x08) { $score++ }
    if ($sizeByte -lt 0x10) { $score++ }
    if ($sramByte -lt 0x08) { $score++ }
    if ((($chk + $comp) -eq 0xFFFF) -and ($chk -ne 0) -and ($comp -ne 0)) { $score += 8 }

    $rvOff = $addr + 0x7FFC
    $resetVec = [int]$rom[$rvOff] + ([int]$rom[$rvOff + 1] * 256)
    if ($resetVec -lt 0x8000) { return -1 }

    $opIdx = $addr + ($resetVec -band 0x7FFF)
    $op = if ($opIdx -lt $romSize) { [int]$rom[$opIdx] } else { 0 }
    if ($op -in @(0x18, 0x78, 0x4C, 0x5C, 0x20, 0x22, 0x9C)) { $score += 8 }   # CLI,SEI,JMP,JML,JSR,JSL,STZ
    elseif ($op -in @(0xC2, 0xE2, 0xA9, 0xA2, 0xA0)) { $score += 4 }           # REP,SEP,LDA,LDX,LDY
    elseif ($op -in @(0x00, 0xFF, 0xCC)) { $score -= 8 }                       # BRK,SBC,CPY
    return [Math]::Max(0, $score)
}

$SRC_DIR = "$ProjectRoot\src"
$BUILD_DIR = "$ProjectRoot\build"
$ROM_FILE = "$BUILD_DIR\$RomName.sfc"
$MAP_FILE = "$BUILD_DIR\$RomName.map"
$SYM_FILE = "$BUILD_DIR\$RomName.sym"

$PVS_INCLUDE = "$PVSNESLIB_HOME\include"
$PVS_LIB_DIR = "$PVSNESLIB_HOME\lib\$Target"

# PVSnesLib object files (pre-built with 816-tcc) - NOT USED (COFF format incompatible with ld65)
# $PVS_OBJECTS = @(
#     "$PVS_LIB_DIR\crt0_snes.obj",
#     "$PVS_LIB_DIR\libc.obj",
#     "$PVS_LIB_DIR\libm.obj",
#     "$PVS_LIB_DIR\libtcc.obj"
# )

# Verify PVSnesLib objects exist - SKIPPED (using custom crt0.s instead)
# foreach ($obj in $PVS_OBJECTS) {
#     if (-not (Test-Path $obj)) {
#         Write-Error "Missing PVSnesLib object: $obj"
#         exit 1
#     }
# }

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
    & $CL65 -t none --cpu 65816 -I"$SRC_DIR" -O -c -o $obj $c
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    $ObjFiles += $obj
}

# --- Assemble ASM files ---
foreach ($asm in $ASMFiles) {
    $obj = "$BUILD_DIR\$([IO.Path]::GetFileNameWithoutExtension($asm)).o"
    Write-Host "Assembling $asm..." -ForegroundColor Green
    & $CA65 --cpu 65816 -I"$SRC_DIR" -o $obj $asm
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    $ObjFiles += $obj
}

foreach ($s in $SFiles) {
    $obj = "$BUILD_DIR\$([IO.Path]::GetFileNameWithoutExtension($s)).o"
    Write-Host "Assembling $s..." -ForegroundColor Green
    & $CA65 --cpu 65816 -I"$SRC_DIR" -o $obj $s
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    $ObjFiles += $obj
}

$LINKER_CFG = "$ScriptDir\snes-lorom.cfg"

# --- Link ---
Write-Host "Linking $ROM_FILE..." -ForegroundColor Green
$linkArgs = @(
    "-t", "none",
    "--cpu", "65816",
    "-C", $LINKER_CFG,
    "-o", $ROM_FILE,
    "-m", $MAP_FILE,
    "-Cl",
    $ObjFiles
)

& $CL65 @linkArgs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

# --- Show header + vectors right after linking ---
# The linker config produces an exactly-32KB image with the SNES header at
# file offset 0x7FC0 and the vector table at 0x7FE0.
$bytes = [System.IO.File]::ReadAllBytes($ROM_FILE)
if ($bytes.Length -ne 0x8000) {
    Write-Error "Linked ROM is $($bytes.Length) bytes; expected exactly 0x8000. Check scripts/snes-lorom.cfg."
    exit 1
}
Write-Host "Header (0x7FC0) + vectors (0x7FE0) after linking:" -ForegroundColor Yellow
($bytes[0x7FC0..0x7FFF] | ForEach-Object { "{0:X2}" -f $_ }) -join ' '

# --- Pad ROM to 256 KB (2 Mbit, matches the header ROM size byte $08) ---
if ($bytes.Length -lt 0x40000) {
    $padded = New-Object byte[] 0x40000
    [Array]::Copy($bytes, $padded, $bytes.Length)
    [System.IO.File]::WriteAllBytes($ROM_FILE, $padded)
    $bytes = $padded
}

# --- Verify ROM ---
$romSize = (Get-Item $ROM_FILE).Length
Write-Host "ROM built: $ROM_FILE ($([math]::Round($romSize/1024,1)) KB)" -ForegroundColor Cyan

# --- Calculate and write SNES header checksum ---
# SNES checksum: 16-bit word sum of all bytes in ROM (excluding checksum bytes at 0x7FDC-0x7FDF)
$sum = 0
for ($i = 0; $i -lt 0x40000; $i += 2) {
    if ($i -eq 0x7FDC -or $i -eq 0x7FDE) { continue }
    $sum += $bytes[$i] + (([int]$bytes[$i+1]) -shl 8)
}
$sum = $sum -band 0xFFFF
$comp = 0xFFFF - $sum
# Checksum at $7FDE-$7FDF (little endian), complement at $7FDC-$7FDD
$bytes[0x7FDC] = $comp -band 0xFF
$bytes[0x7FDD] = ($comp -shr 8) -band 0xFF
$bytes[0x7FDE] = $sum -band 0xFF
$bytes[0x7FDF] = ($sum -shr 8) -band 0xFF
[System.IO.File]::WriteAllBytes($ROM_FILE, $bytes)
Write-Host "Checksum: $([string]::Format("{0:X4}", $sum))  Complement: $([string]::Format("{0:X4}", $comp))" -ForegroundColor Cyan

# --- Validate the image using Mesen-S's exact header-scoring logic ---
# Replicates BaseCartridge::LoadRom (Mesen-S Core/BaseCartridge.cpp): the
# emulator scores candidate base addresses and accepts the best score >= 0.
$bestScore = -1
$isLoRom = $true
$hasCopierHeader = $false
$hdrOff = 0
foreach ($base in @(0, 0x200, 0x8000, 0x8200, 0x408000, 0x408200)) {
    $score = Get-SnesHeaderScore $bytes $base
    if ($score -ge 0 -and $score -ge $bestScore) {
        $bestScore = $score
        $isLoRom = (($base -band 0x8000) -eq 0)
        $hasCopierHeader = (($base -band 0x200) -ne 0)
        $hdrOff = [Math]::Min($base + 0x7FB0, $bytes.Length - 80)
    }
}

$title = ([System.Text.Encoding]::ASCII.GetString($bytes, $hdrOff + 16, 21)).TrimEnd()
$mapMode = $bytes[$hdrOff + 37]

Write-Host "Mesen-S header scoring: best score $bestScore at file offset 0x$('{0:X}' -f $hdrOff)" -ForegroundColor Yellow
Write-Host "  Title      : $title"
Write-Host ("  Map mode   : 0x{0:X2}" -f $mapMode)
Write-Host ("  Mapping    : {0}" -f $(if ($isLoRom) { "LoROM" } else { "HiROM" }))

$errors = @()
if ($bytes.Length -ne 0x40000) { $errors += "File size is 0x$('{0:X}' -f $bytes.Length), expected 0x40000 (2 Mbit)" }
if ($bestScore -lt 0) { $errors += "Mesen-S would REJECT this ROM: no valid header candidate found" }
if (-not $isLoRom) { $errors += "Mesen-S detected HiROM, expected LoROM" }
if ($hasCopierHeader) { $errors += "Mesen-S thinks there is a 512-byte copier header" }
if ($hdrOff -ne 0x7FB0) { $errors += "Header not at file offset 0x7FC0" }
if ($bestScore -lt 12) { $errors += "Header score $bestScore too low; expected >= 12 (mode/type/size/checksum/reset bonuses)" }

if ($errors.Count -gt 0) {
    $errors | ForEach-Object { Write-Error $_ }
    exit 1
}
Write-Host "ROM passes Mesen-S's exact header validation (tools/verify_rom.py = same check in Python)." -ForegroundColor Green

# --- Run in emulator ---
if ($Run) {
    $emuPath = @(
        "C:\dev\snes\tools\mesen-s\Mesen-S.exe",
        "$env:LOCALAPPDATA\Microsoft\WinGet\Packages\SourMesen.Mesen-S_Microsoft.Winget.Source_8wekyb3d8bbwe\Mesen-S.exe",
        "C:\Program Files\Mesen-S\Mesen-S.exe",
        "C:\Program Files (x86)\Mesen-S\Mesen-S.exe",
        "$env:LOCALAPPDATA\Mesen-S\Mesen-S.exe",
        "C:\Program Files\bsnes\bsnes.exe",
        "C:\Program Files\snes9x\snes9x.exe",
        "C:\Users\ramga\Source\star-merchants-snes\snes9x\snes9x.exe"
    ) | Where-Object { Test-Path $_ } | Select-Object -First 1

    if ($emuPath) {
        Write-Host "Launching $emuPath..." -ForegroundColor Cyan
        & $emuPath $ROM_FILE
    } else {
        Write-Warning "No emulator found. Install Mesen-S, bsnes, or snes9x."
    }
}