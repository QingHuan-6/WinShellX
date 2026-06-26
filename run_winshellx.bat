@echo off
cd /d "%~dp0"

set "EXE=build\Debug\WinShellX.exe"
if not exist "%EXE%" (
    echo WinShellX.exe not found at %EXE%
    echo Please run build.bat first.
    pause
    exit /b 1
)

"%EXE%"
echo.
echo WinShellX has exited.
pause
