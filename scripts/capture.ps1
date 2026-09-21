# Captures the Mesen-S window to a PNG for automated visual debugging.
param(
    [string]$Out = "build\shot.png",
    [int]$DelaySeconds = 3
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
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);
    public struct RECT { public int Left; public int Top; public int Right; public int Bottom; }
    public struct POINT { public int X; public int Y; }
}
"@

$proc = Get-Process -Name "Mesen-S" -ErrorAction SilentlyContinue | Where-Object { $_.MainWindowHandle -ne 0 } | Select-Object -First 1
if (-not $proc) { Write-Error "Mesen-S window not found"; exit 1 }

[Win32Cap]::ShowWindow($proc.MainWindowHandle, 9) | Out-Null   # SW_RESTORE
[Win32Cap]::SetForegroundWindow($proc.MainWindowHandle) | Out-Null
Start-Sleep -Milliseconds 500

# Get client area (game screen) not window frame
$rect = New-Object Win32Cap+RECT
[Win32Cap]::GetClientRect($proc.MainWindowHandle, [ref]$rect) | Out-Null
$w = $rect.Right - $rect.Left
$h = $rect.Bottom - $rect.Top
if ($w -le 0 -or $h -le 0) { Write-Error "Invalid client rect"; exit 1 }

# Convert client top-left to screen coordinates
$pt = New-Object Win32Cap+POINT
$pt.X = $rect.Left
$pt.Y = $rect.Top
[Win32Cap]::ClientToScreen($proc.MainWindowHandle, [ref]$pt) | Out-Null

Start-Sleep -Seconds $DelaySeconds
$bmp = New-Object System.Drawing.Bitmap $w, $h
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.CopyFromScreen($pt.X, $pt.Y, 0, 0, $bmp.Size)
$g.Dispose()
$bmp.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
$bmp.Dispose()
Write-Host "Captured client area $w x $h -> $Out"

# Close Mesen-S after successful capture
$proc.CloseMainWindow() | Out-Null
Start-Sleep -Milliseconds 500
if (-not $proc.HasExited) { $proc.Kill() | Out-Null }
Write-Host "Closed Mesen-S (PID $($proc.Id))"
