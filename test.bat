@echo off
setlocal EnableExtensions

cd /d "%~dp0"

set "BUILD_DIR=build"
set "CONFIG=Debug"
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

echo [ERROR] cmake not found.
pause
exit /b 1

:found_cmake
echo ========================================
echo  WinShellX Tests
echo ========================================
echo.

"%CMAKE%" -S . -B "%BUILD_DIR%"
if errorlevel 1 goto :failed

"%CMAKE%" --build "%BUILD_DIR%" --config %CONFIG% --target WinShellX_tests
if errorlevel 1 goto :failed

echo.
"%BUILD_DIR%\%CONFIG%\WinShellX_tests.exe"
set "TEST_EXIT=%ERRORLEVEL%"

echo.
if not "%TEST_EXIT%"=="0" (
    echo [ERROR] Tests failed.
    pause
    exit /b %TEST_EXIT%
)

echo All tests passed.
pause
exit /b 0

:failed
echo [ERROR] Test build failed.
pause
exit /b 1
