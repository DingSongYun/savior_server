@echo off
echo ========================================
echo Savior Server - Boost Headers Setup
echo ========================================
echo.

set "PROJECT_ROOT=%~dp0"
set "LIB_DIR=%PROJECT_ROOT%lib"
set "BOOST_DIR=%LIB_DIR%\boost"

echo Setting up minimal Boost headers...
echo Target directory: %BOOST_DIR%
echo.

REM 清理并创建目录
if exist "%BOOST_DIR%" (
    echo Removing existing Boost directory...
    rmdir /s /q "%BOOST_DIR%" 2>nul
)

echo Creating directory structure...
mkdir "%BOOST_DIR%"
mkdir "%BOOST_DIR%\boost"
mkdir "%BOOST_DIR%\boost\system"

echo.
echo Creating minimal Boost headers for testing...

REM 创建 config.hpp
(
echo // Minimal Boost configuration
echo #ifndef BOOST_CONFIG_HPP
echo #define BOOST_CONFIG_HPP
echo.
echo #ifdef _WIN32
echo #define BOOST_WINDOWS_API
echo #endif
echo.
echo #ifdef _MSC_VER
echo #define BOOST_MSVC _MSC_VER
echo #endif
echo.
echo #endif
) > "%BOOST_DIR%\boost\config.hpp"

REM 创建 version.hpp
(
echo // Boost version
echo #ifndef BOOST_VERSION_HPP
echo #define BOOST_VERSION_HPP
echo.
echo #define BOOST_VERSION 108400
echo #define BOOST_LIB_VERSION "1_84"
echo.
echo #endif
) > "%BOOST_DIR%\boost\version.hpp"

REM 创建 error_code.hpp
(
echo // Simple error_code for testing
echo #ifndef BOOST_SYSTEM_ERROR_CODE_HPP
echo #define BOOST_SYSTEM_ERROR_CODE_HPP
echo.
echo #include ^<string^>
echo.
echo namespace boost {
echo namespace system {
echo.
echo class error_code {
echo public:
echo     error_code^(^) : value_^(0^) {}
echo     explicit error_code^(int val^) : value_^(val^) {}
echo     operator bool^(^) const { return value_ != 0; }
echo     std::string message^(^) const { return value_ ? "Error" : "Success"; }
echo private:
echo     int value_;
echo };
echo.
echo }
echo }
echo.
echo #endif
) > "%BOOST_DIR%\boost\system\error_code.hpp"

REM 创建简化的 asio.hpp
(
echo // Simplified Asio for testing
echo #ifndef BOOST_ASIO_HPP
echo #define BOOST_ASIO_HPP
echo.
echo #include ^<iostream^>
echo #include ^<chrono^>
echo #include ^<functional^>
echo #include "system/error_code.hpp"
echo.
echo namespace boost {
echo namespace asio {
echo.
echo class io_context {
echo public:
echo     void run^(^) { 
echo         std::cout ^<^< "Mock io_context running..." ^<^< std::endl; 
echo     }
echo };
echo.
echo class steady_timer {
echo public:
echo     template^<typename Duration^>
echo     steady_timer^(io_context^& io, Duration d^) {}
echo     
echo     template^<typename Handler^>
echo     void async_wait^(Handler h^) { 
echo         h^(boost::system::error_code^(^)^); 
echo     }
echo };
echo.
echo }
echo }
echo.
echo #endif
) > "%BOOST_DIR%\boost\asio.hpp"

echo.
echo Verifying installation...
if exist "%BOOST_DIR%\boost\asio.hpp" (
    echo ✓ Boost.Asio header created
) else (
    echo ✗ Failed to create Boost.Asio header
)

if exist "%BOOST_DIR%\boost\system\error_code.hpp" (
    echo ✓ Boost.System header created
) else (
    echo ✗ Failed to create Boost.System header
)

if exist "%BOOST_DIR%\boost\version.hpp" (
    echo ✓ Boost version header created
) else (
    echo ✗ Failed to create Boost version header
)

echo.
echo ========================================
echo Boost headers setup completed!
echo ========================================
echo.
echo Location: %BOOST_DIR%
echo.
echo Note: This is a minimal test setup.
echo For production, use the full Boost library.
echo.
echo Next steps:
echo 1. Run build.bat to test compilation
echo 2. Run test_boost_config.exe to verify
echo.
pause