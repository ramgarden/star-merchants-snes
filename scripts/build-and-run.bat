@echo off
setlocal

set "MSYS_ROOT=C:\msys64"
set "PVSNESLIB_HOME=/c/dev/snes/tools/pvsneslib"
set "PATH=%MSYS_ROOT%\usr\bin;%MSYS_ROOT%\ucrt64\bin;%PATH%"

cd /d "%~dp0..\game"

echo.
echo ===========================
echo Cleaning and rebuilding...
echo ===========================
echo.

call make clean
if errorlevel 1 (
    echo.
    echo Clean FAILED.
    pause
    exit /b 1
)

call make
if errorlevel 1 (
    echo.
    echo Build FAILED.
    pause
    exit /b 1
)

echo.
echo ===========================
echo Launching Mesen...
echo ===========================
echo.

taskkill /F /IM Mesen.exe >nul 2>&1
start "" "C:\dev\snes\tools\mesen\Mesen.exe" "%CD%\star_merchants.sfc"