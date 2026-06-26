@echo off
setlocal EnableExtensions

cd /d "%~dp0"

set "BUILD_DIR=build"
set "CONFIG=Debug"
set "EXE_OUT=%BUILD_DIR%\%CONFIG%\WinShellX.exe"
set "CMAKE="

where cmake >nul 2>&1
if not errorlevel 1 (
    set "CMAKE=cmake"
    goto :found_cmake
)

if exist "D:\visualstudio\ide\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
    set "CMAKE=D:\visualstudio\ide\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    goto :found_cmake
)

if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
    set "CMAKE=%ProgramFiles%\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    goto :found_cmake
)

echo [ERROR] cmake not found.
echo Install Visual Studio with "Desktop development with C++" and CMake tools,
echo or add cmake to PATH.
pause
exit /b 1

:found_cmake
echo ========================================
echo  WinShellX Build (%CONFIG%)
echo ========================================
echo.

echo [1/3] Configure CMake...
"%CMAKE%" -S . -B "%BUILD_DIR%"
if errorlevel 1 goto :failed

echo.
echo [2/3] Build...
"%CMAKE%" --build "%BUILD_DIR%" --config %CONFIG%
if errorlevel 1 goto :failed

if not exist "%EXE_OUT%" (
    echo [ERROR] Build finished but exe not found: %EXE_OUT%
    pause
    exit /b 1
)

echo.
echo [3/3] Copy WinShellX.exe to project root...
copy /Y "%EXE_OUT%" "WinShellX.exe" >nul
if errorlevel 1 (
    echo [WARN] Could not copy to project root. Is WinShellX.exe still running?
    echo        You can run: %EXE_OUT%
) else (
    echo Copied to: %CD%\WinShellX.exe
)

echo.
echo ========================================
echo  Build succeeded!
echo  Output: %CD%\WinShellX.exe
echo  Run:     run_winshellx.bat
echo ========================================
echo.
pause
exit /b 0

:failed
echo.
echo [ERROR] Build failed.
pause
exit /b 1
