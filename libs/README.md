# Boost 源码集成指南

本项目支持将 Boost 以源码形式集成到项目中，确保完全的自包含性和版本控制。

## 📁 目录结构

```
savior_server/
├── lib/
│   └── boost/                  # Boost 源码目录
│       ├── boost/              # Boost 头文件
│       ├── libs/               # Boost 库源码
│       └── ...
├── setup_boost_source.bat      # Boost 源码下载脚本
└── ...
```

## 🚀 快速设置

### 1. 下载 Boost 源码

运行自动下载脚本：

```bash
# 在 savior_server 根目录下运行
.\setup_boost_source.bat
```

这个脚本会：
- 下载 Boost 1.84.0 源码
- 解压到 `lib/boost` 目录
- 自动清理临时文件

### 2. 验证安装

检查 `lib/boost` 目录是否包含：
- `boost/` 目录（头文件）
- `libs/` 目录（库源码）
- `bootstrap.bat` 文件

### 3. 构建项目

```bash
# 主项目构建
cd build
.\build.bat

# 或者 boost_asio_learning 项目
cd examples\boost_asio_learning\build
.\build.bat
```

## 🔧 技术细节

### CMake 配置

项目的 CMakeLists.txt 文件已配置为：

1. **优先使用本地源码**：首先检查 `lib/boost` 目录
2. **Header-only 模式**：Boost.Asio 是纯头文件库，无需编译
3. **自动回退**：如果本地源码不存在，尝试查找系统安装的 Boost
4. **清晰提示**：提供明确的设置指导

### 支持的 Boost 库

当前配置主要支持：
- **Boost.Asio** - 网络和异步 I/O（header-only）
- **Boost.System** - 系统错误处理
- **Boost.Thread** - 线程支持

## 📋 优势

### ✅ 完全自包含
- 项目包含所有依赖
- 无需系统级 Boost 安装
- 版本控制友好

### ✅ 版本一致性
- 固定 Boost 版本（1.84.0）
- 避免版本冲突
- 可重现的构建

### ✅ 简化部署
- 单一项目目录
- 无外部依赖
- 跨环境一致性

## 🛠️ 手动设置（可选）

如果自动脚本失败，可以手动设置：

1. **下载 Boost**：
   - 访问 [boost.org](https://www.boost.org/users/download/)
   - 下载 Boost 1.84.0 源码

2. **解压到正确位置**：
   ```
   savior_server/lib/boost/
   ```

3. **验证目录结构**：
   确保 `lib/boost/boost/asio.hpp` 文件存在

## 🔍 故障排除

### 问题：CMake 找不到 Boost
**解决方案**：
- 检查 `lib/boost` 目录是否存在
- 验证 `lib/boost/boost/asio.hpp` 文件存在
- 重新运行 `setup_boost_source.bat`

### 问题：编译错误
**解决方案**：
- 确保使用 C++17 标准
- 检查 Visual Studio 2022 安装
- 查看具体错误信息

### 问题：下载失败
**解决方案**：
- 检查网络连接
- 尝试手动下载
- 使用代理或 VPN

## 📚 相关文档

- [Boost.Asio 官方文档](https://www.boost.org/doc/libs/1_84_0/doc/html/boost_asio.html)
- [项目学习指南](docs/learning_guide.md)
- [构建系统说明](README.md)

---

**注意**：本配置主要针对 Boost.Asio 的学习和使用。如果需要其他 Boost 库的编译版本，可能需要额外的构建步骤。