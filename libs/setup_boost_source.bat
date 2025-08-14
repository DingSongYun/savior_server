@echo off
echo ========================================
echo Savior Server - Boost Source Setup
echo ========================================
echo.

set "PROJECT_ROOT=%~dp0"
set "LIB_DIR=%PROJECT_ROOT%lib"
set "BOOST_DIR=%LIB_DIR%\boost"

echo Setting up Boost 1.84.0 source code...
echo Project root: %PROJECT_ROOT%
echo Target directory: %BOOST_DIR%
echo.

REM 创建 lib 目录
if not exist "%LIB_DIR%" (
    echo Creating lib directory...
    mkdir "%LIB_DIR%"
)

REM 检查是否已经存在 Boost
if exist "%BOOST_DIR%" (
    echo Boost directory already exists at: %BOOST_DIR%
    echo Checking for key header files...
    
    if exist "%BOOST_DIR%\boost\asio.hpp" (
        echo ✓ Boost.Asio header found
    ) else (
        echo ✗ Boost.Asio header not found
    )
    
    if exist "%BOOST_DIR%\boost\system\error_code.hpp" (
        echo ✓ Boost.System header found
    ) else (
        echo ✗ Boost.System header not found
    )
    
    echo.
    echo If headers are missing, delete %BOOST_DIR% and run this script again.
    pause
    exit /b 0
)

echo Creating Boost directory...
mkdir "%BOOST_DIR%"

echo.
echo Downloading Boost 1.84.0 source...
echo This may take a few minutes...
echo.

REM 使用 PowerShell 下载 Boost
powershell -Command "$ProgressPreference = 'SilentlyContinue'; $url = 'https://boostorg.jfrog.io/artifactory/main/release/1.84.0/source/boost_1_84_0.tar.gz'; $output = '%LIB_DIR%\boost_1_84_0.tar.gz'; Write-Host 'Downloading from:' $url; Write-Host 'Saving to:' $output; try { Invoke-WebRequest -Uri $url -OutFile $output -UseBasicParsing; Write-Host 'Download completed successfully' } catch { Write-Host 'Download failed:' $_.Exception.Message; exit 1 }"

if %ERRORLEVEL% neq 0 (
    echo Download failed!
    pause
    exit /b 1
)

echo.
echo Extracting Boost archive...

REM 使用 PowerShell 解压 tar.gz
powershell -Command "$archive = '%LIB_DIR%\boost_1_84_0.tar.gz'; $extractPath = '%LIB_DIR%'; Write-Host 'Extracting:' $archive; Write-Host 'To:' $extractPath; try { Add-Type -AssemblyName System.IO.Compression.FileSystem; $gzipStream = [System.IO.File]::OpenRead($archive); $gzipReader = New-Object System.IO.Compression.GzipStream($gzipStream, [System.IO.Compression.CompressionMode]::Decompress); $tarFile = '%LIB_DIR%\boost_1_84_0.tar'; $tarStream = [System.IO.File]::Create($tarFile); $gzipReader.CopyTo($tarStream); $tarStream.Close(); $gzipReader.Close(); $gzipStream.Close(); Write-Host 'Gzip extraction completed'; $tarCmd = 'tar -xf \"' + $tarFile + '\" -C \"' + $extractPath + '\"'; Write-Host 'Running:' $tarCmd; cmd /c $tarCmd; if ($LASTEXITCODE -eq 0) { Write-Host 'Tar extraction completed' } else { Write-Host 'Tar extraction failed'; exit 1 }; Remove-Item $tarFile -Force } catch { Write-Host 'Extraction failed:' $_.Exception.Message; exit 1 }"

if %ERRORLEVEL% neq 0 (
    echo Extraction failed!
    pause
    exit /b 1
)

echo.
echo Moving Boost files to correct location...

REM 移动解压的文件到正确位置
if exist "%LIB_DIR%\boost_1_84_0" (
    echo Moving boost_1_84_0 to boost...
    move "%LIB_DIR%\boost_1_84_0" "%BOOST_DIR%"
    if %ERRORLEVEL% neq 0 (
        echo Failed to move Boost directory!
        pause
        exit /b 1
    )
) else (
    echo Error: boost_1_84_0 directory not found after extraction!
    pause
    exit /b 1
)

echo.
echo Cleaning up temporary files...
if exist "%LIB_DIR%\boost_1_84_0.tar.gz" (
    del "%LIB_DIR%\boost_1_84_0.tar.gz"
)

echo.
echo Verifying Boost installation...
if exist "%BOOST_DIR%\boost\asio.hpp" (
    echo ✓ Boost.Asio header found
) else (
    echo ✗ Boost.Asio header not found
)

if exist "%BOOST_DIR%\boost\system\error_code.hpp" (
    echo ✓ Boost.System header found
) else (
    echo ✗ Boost.System header not found
)

if exist "%BOOST_DIR%\boost\version.hpp" (
    echo ✓ Boost version header found
) else (
    echo ✗ Boost version header not found
)

echo.
echo ========================================
echo Boost source setup completed!
echo ========================================
echo.
echo Boost source location: %BOOST_DIR%
echo.
echo Next steps:
echo 1. Run build.bat to compile the project
echo 2. Run test_boost_config.exe to verify Boost configuration
echo.
pause