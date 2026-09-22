# Captures the Mesen-S window to a PNG for automated visual debugging.
# Uses a real screen copy (CopyFromScreen) so GPU-rendered game pixels are
# captured — PrintWindow only sees GDI chrome and returns black for the game
# surface. A screen copy is only correct if the window is actually on top, so
# the script forces z-order/focus and verifies it before shooting.
param(
    [string]$Out = "build\shot.png",
    [int]$DelaySeconds = 3,
    [switch]$ClientOnly,
    [switch]$NoClose,
    [switch]$UsePrintWindow,
    [string]$ProcessName = "Mesen-S"
)
Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing
Add-Type @"
using System;
using System.Runtime.InteropServices;
public class Win32Cap {
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT rect);
    [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr hWnd, out RECT rect);
    [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr hWnd, ref POINT pt);
    [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr hWnd, IntPtr hdcBlt, uint nFlags);
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
    [DllImport("user32.dll")] public static extern IntPtr SetFocus(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);
    [DllImport("user32.dll")] public static extern bool IsWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool IsIconic(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr hWnd, IntPtr hWndInsertAfter, int X, int Y, int cx, int cy, uint uFlags);
    [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
    public struct RECT { public int Left; public int Top; public int Right; public int Bottom; }
    public struct POINT { public int X; public int Y; }
}
"@

# Align coordinate spaces: without this, GetWindowRect returns logical pixels
# while CopyFromScreen works in physical pixels, and captures get cropped on
# scaled (>100%) displays. May return false under a manifest; harmless.
[Win32Cap]::SetProcessDPIAware() | Out-Null

function Find-EmuWindow([string]$Name) {
    return Get-Process -Name $Name -ErrorAction SilentlyContinue |
        Where-Object { $_.MainWindowHandle -ne 0 -and [Win32Cap]::IsWindow($_.MainWindowHandle) } |
        Select-Object -First 1
}

$proc = Find-EmuWindow $ProcessName
if (-not $proc) { Write-Error "$ProcessName window not found"; exit 1 }
$hWnd = $proc.MainWindowHandle

# 1. Restore if minimized — a minimized window has no paintable pixels.
if ([Win32Cap]::IsIconic($hWnd)) {
    [Win32Cap]::ShowWindow($hWnd, 9) | Out-Null   # SW_RESTORE
    Start-Sleep -Milliseconds 500
}

# 2. Force z-order to the front. SetForegroundWindow alone is routinely
# rejected by the foreground-lock (our process is not foreground), so toggle
# TOPMOST on and back off: this reliably raises the window above all other
# non-topmost windows without leaving it pinned on top.
$HWND_TOPMOST = [IntPtr](-1)
$HWND_NOTOPMOST = [IntPtr](-2)
$SWP = 0x0001 -bor 0x0002 -bor 0x0040  # NOSIZE | NOMOVE | SHOWWINDOW
[Win32Cap]::SetWindowPos($hWnd, $HWND_TOPMOST, 0, 0, 0, 0, $SWP) | Out-Null
[Win32Cap]::SetWindowPos($hWnd, $HWND_NOTOPMOST, 0, 0, 0, 0, $SWP) | Out-Null

# 3. Ask for input focus too, and poll until foreground (or timeout).
# Focus is best-effort: pixel correctness only needs z-order, which step 2
# already handled, so a failure here is a warning, not an error.
[Win32Cap]::SetForegroundWindow($hWnd) | Out-Null
[Win32Cap]::SetFocus($hWnd) | Out-Null
$fgOk = $false
for ($i = 0; $i -lt 50; $i++) {
    if ([Win32Cap]::GetForegroundWindow() -eq $hWnd) { $fgOk = $true; break }
    Start-Sleep -Milliseconds 100
    [Win32Cap]::SetForegroundWindow($hWnd) | Out-Null
}
if (-not $fgOk) {
    Write-Warning "$ProcessName is topmost in z-order but did not receive input focus; capture proceeds."
}

# 4. Let the game render frames once the window is placed.
Start-Sleep -Seconds $DelaySeconds

# 5. Re-resolve the window and measure FRESH bounds — never reuse
# coordinates from before the restore/focus dance above.
$proc.Refresh()
$proc = Find-EmuWindow $ProcessName
if (-not $proc) { Write-Error "$ProcessName window disappeared before capture"; exit 1 }
$hWnd = $proc.MainWindowHandle
if (-not [Win32Cap]::IsWindowVisible($hWnd)) { Write-Error "$ProcessName window is not visible"; exit 1 }

$outDir = Split-Path -Parent $Out
if ($outDir -and -not (Test-Path -LiteralPath $outDir)) {
    New-Item -ItemType Directory -Path $outDir | Out-Null
}

if ($UsePrintWindow) {
    # Occlusion-proof but GDI-only: title bar/menus capture, GPU game
    # surface comes back black. Chrome debugging only.
    $rect = New-Object Win32Cap+RECT
    if ($ClientOnly) {
        [Win32Cap]::GetClientRect($hWnd, [ref]$rect) | Out-Null
    } else {
        [Win32Cap]::GetWindowRect($hWnd, [ref]$rect) | Out-Null
    }
    $w = $rect.Right - $rect.Left
    $h = $rect.Bottom - $rect.Top
    if ($w -le 0 -or $h -le 0) { Write-Error "Invalid window rect ${w}x${h}"; exit 1 }
    $bmp = New-Object System.Drawing.Bitmap $w, $h
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $hdc = $g.GetHdc()
    try {
        $flags = if ($ClientOnly) { 1 } else { 0 }  # 1 = PW_CLIENTONLY
        if (-not [Win32Cap]::PrintWindow($hWnd, $hdc, $flags)) {
            Write-Error "PrintWindow failed for HWND $hWnd"
            exit 1
        }
    } finally {
        $g.ReleaseHdc($hdc)
        $g.Dispose()
    }
} else {
    if ($ClientOnly) {
        $rect = New-Object Win32Cap+RECT
        [Win32Cap]::GetClientRect($hWnd, [ref]$rect) | Out-Null
        $w = $rect.Right - $rect.Left
        $h = $rect.Bottom - $rect.Top
        if ($w -le 0 -or $h -le 0) { Write-Error "Invalid client rect ${w}x${h}"; exit 1 }
        $pt = New-Object Win32Cap+POINT
        $pt.X = $rect.Left
        $pt.Y = $rect.Top
        [Win32Cap]::ClientToScreen($hWnd, [ref]$pt) | Out-Null
        $sx = $pt.X
        $sy = $pt.Y
    } else {
        $rect = New-Object Win32Cap+RECT
        [Win32Cap]::GetWindowRect($hWnd, [ref]$rect) | Out-Null
        $w = $rect.Right - $rect.Left
        $h = $rect.Bottom - $rect.Top
        if ($w -le 0 -or $h -le 0) { Write-Error "Invalid window rect ${w}x${h}"; exit 1 }
        $sx = $rect.Left
        $sy = $rect.Top
    }
    $bmp = New-Object System.Drawing.Bitmap $w, $h
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    try {
        $g.CopyFromScreen($sx, $sy, 0, 0, $bmp.Size)
    } finally {
        $g.Dispose()
    }
}
$bmp.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
$bmp.Dispose()
$mode = if ($ClientOnly) { "client area" } else { "full window" }
Write-Host "Captured $mode $w x $h -> $Out"

if (-not $NoClose) {
    # Close Mesen-S after successful capture
    $proc.CloseMainWindow() | Out-Null
    Start-Sleep -Milliseconds 500
    $proc.Refresh()
    if (-not $proc.HasExited) { $proc.Kill() | Out-Null }
    Write-Host "Closed $ProcessName (PID $($proc.Id))"
}
