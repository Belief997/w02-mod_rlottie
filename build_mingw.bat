@echo off
REM MinGW Build Script for lottie_renderer
REM Usage: build_mingw.bat [Release|Debug]

setlocal

set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release

echo ========================================
echo Building lottie_renderer with MinGW
echo Build Type: %BUILD_TYPE%
echo ========================================

REM Create build directory
if not exist build_mingw mkdir build_mingw

REM Configure with CMake
echo.
echo [1/3] Configuring CMake...
cmake -G "MinGW Makefiles" -B build_mingw -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DLOTTIE_BUILD_TEST=ON

if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    exit /b 1
)

REM Build
echo.
echo [2/3] Building...
cmake --build build_mingw --config %BUILD_TYPE% -j%NUMBER_OF_PROCESSORS%

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)

echo.
echo [3/3] Build Complete!
echo.
echo Output files:
echo   Library: build_mingw\liblottie_renderer.a
echo   C Test:  build_mingw\test\c_test\lottie_c_test.exe
echo   C++ Test: build_mingw\test\win\lottie_test.exe
echo.
echo Usage:
echo   build_mingw\test\c_test\lottie_c_test.exe animation.json --play
echo   build_mingw\test\c_test\lottie_c_test.exe animation.json --save output

endlocal
