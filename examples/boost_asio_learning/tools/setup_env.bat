@echo off
REM Sarior Server - Environment Setup Script

echo ========================================
echo Sarior Server Environment Setup
echo ========================================

REM Check Visual Studio 2022
echo Checking Visual Studio 2022...
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    echo [OK] Found Visual Studio 2022 Community
) else (
    echo [ERROR] Visual Studio 2022 Community not found
    echo Please download and install from:
    echo https://visualstudio.microsoft.com/vs/community/
    goto :error
)

REM Check CMake
echo Checking CMake...
cmake --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] CMake not found
    echo Please download and install from:
    echo https://cmake.org/download/
    goto :error
) else (
    echo [OK] Found CMake
)

REM Check Git
echo Checking Git...
git --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Git not found
    echo Please download and install from:
    echo https://git-scm.com/download/win
    goto :error
) else (
    echo [OK] Found Git
)

REM Setup Visual Studio environment
echo.
echo Setting up Visual Studio environment...
call "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 goto :error

REM Check compiler
echo Checking compiler...
cl >nul 2>&1
if errorlevel 1 (
    echo [ERROR] cl.exe not available
    goto :error
) else (
    echo [OK] cl.exe is available
)

echo.
echo ========================================
echo Environment setup completed!
echo ========================================
echo.
echo Now you can run the build script:
echo   cd build
echo   build.bat
echo.
goto :end

:error
echo.
echo ========================================
echo Environment setup failed!
echo ========================================
pause
exit /b 1

:end
pause