# Boost Library Installation Guide

This guide will help you install the Boost C++ libraries to enable full functionality of the Sarior Server project.

## Option 1: Using vcpkg (Recommended)

vcpkg is Microsoft's C++ package manager and the easiest way to install Boost on Windows.

### Step 1: Install vcpkg

```bash
# Clone vcpkg repository
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# Bootstrap vcpkg
.\bootstrap-vcpkg.bat

# Integrate with Visual Studio
.\vcpkg integrate install
```

### Step 2: Install Boost

```bash
# Install Boost for x64 architecture
.\vcpkg install boost:x64-windows

# Or install specific components only
.\vcpkg install boost-system:x64-windows boost-thread:x64-windows boost-asio:x64-windows
```

### Step 3: Configure CMake

Add the vcpkg toolchain file to your CMake configuration:

```bash
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake
```

## Option 2: Download Pre-built Binaries

### Step 1: Download Boost

1. Visit [Boost Downloads](https://www.boost.org/users/download/)
2. Download the latest version for Windows
3. Extract to a folder like `C:\boost`

### Step 2: Build Boost (if needed)

If you downloaded source code:

```bash
cd C:\boost
.\bootstrap.bat
.\b2 --build-type=complete --with-system --with-thread
```

### Step 3: Set Environment Variables

Add these environment variables:
- `BOOST_ROOT`: `C:\boost`
- `BOOST_LIBRARYDIR`: `C:\boost\stage\lib`

## Option 3: Using Conan Package Manager

```bash
# Install Conan
pip install conan

# Create conanfile.txt in project root
echo "[requires]" > conanfile.txt
echo "boost/1.82.0" >> conanfile.txt
echo "[generators]" >> conanfile.txt
echo "cmake" >> conanfile.txt

# Install dependencies
conan install . --build=missing
```

## Verification

After installation, run the build script again:

```bash
cd build
.\build.bat
```

You should see output like:
```
-- Found Boost 1.82.0
-- Added example: 01_basic_concepts
-- Added example: 02_timers
-- Added project: chat_server
-- Added project: chat_client
```

## Troubleshooting

### CMake can't find Boost

1. Make sure `BOOST_ROOT` environment variable is set
2. Try specifying Boost paths manually:
   ```bash
   cmake .. -DBOOST_ROOT=C:\boost -DBoost_INCLUDE_DIR=C:\boost
   ```

### Linking errors

1. Ensure you're using the correct architecture (x64)
2. Check that Boost was built with the same compiler version
3. Verify library paths in `BOOST_LIBRARYDIR`

### vcpkg integration issues

1. Run `.\vcpkg integrate install` as administrator
2. Restart Visual Studio after integration
3. Use the vcpkg toolchain file in CMake

## Next Steps

Once Boost is installed successfully:

1. Run `.\build.bat` to build all examples and projects
2. Explore the examples in `examples/` directory
3. Try the chat server demo in `projects/chat_server/`
4. Read the learning guide in `docs/learning_guide.md`

For more help, see the main README.md or create an issue in the project repository.