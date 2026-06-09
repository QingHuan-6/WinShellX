@echo off
cd /d "%~dp0"
if not exist "WinShellX.exe" (
    echo WinShellX.exe not found.
    echo Please build it first with cl or CMake.
    pause
    exit /b 1
)

WinShellX.exe
echo.
echo WinShellX has exited.
pause
