# Lottie Renderer 编译指南

本文档详细说明如何在不同平台和编译器下构建 lottie_renderer 库。

## 目录

- [环境要求](#环境要求)
- [Windows - Visual Studio](#windows---visual-studio)
- [Windows - MinGW](#windows---mingw)
- [Windows - CMake + MSVC](#windows---cmake--msvc)
- [Linux](#linux)
- [ARM 交叉编译](#arm-交叉编译)
- [编译选项](#编译选项)
- [输出文件](#输出文件)
- [常见问题](#常见问题)

---

## 环境要求

### 通用要求
- CMake 3.10 或更高版本
- C++14 兼容编译器
- C11 兼容编译器 (用于 C 测试程序)

### Windows
- Visual Studio 2019/2022 或
- MinGW-w64 (GCC 8.0+)

### Linux
- GCC 7.0+ 或 Clang 5.0+
- pthread 库

### ARM
- ARM 交叉编译工具链
- 支持 NEON 指令集 (可选，用于优化)

---

## Windows - Visual Studio

### 方法 1: 直接打开解决方案 (推荐)

1. 打开 Visual Studio 2019 或更高版本

2. 打开解决方案文件：
   ```
   lottie_renderer/vs2019/lottie_renderer.sln
   ```

3. 选择配置：
   - 配置: `Debug` 或 `Release`
   - 平台: `x86` 或 `x64`

4. 构建解决方案：
   - 菜单: `生成` → `生成解决方案`
   - 或按 `Ctrl+Shift+B`

5. 输出文件位置：
   ```
   vs2019/x64/Release/lottie_renderer.lib    # 静态库
   vs2019/x64/Release/lottie_test.exe        # C++ 测试程序
   vs2019/x64/Release/lottie_c_test.exe      # C 测试程序
   ```

### 解决方案包含的项目

| 项目 | 类型 | 说明 |
|------|------|------|
| lottie_renderer | 静态库 | 核心渲染库 |
| lottie_test | 可执行文件 | C++ 测试程序 |
| lottie_c_test | 可执行文件 | C 测试程序 |

---

## Windows - MinGW

### 前提条件

1. 安装 MinGW-w64：
   - 下载: https://www.mingw-w64.org/
   - 推荐版本: x86_64-posix-sjlj 或 x86_64-posix-seh

2. 确保 MinGW 在 PATH 中：
   ```batch
   set PATH=C:\mingw64\bin;%PATH%
   gcc --version
   ```

### 方法 1: 使用构建脚本

```batch
# 进入项目目录
cd lottie_renderer

# Release 构建
build_mingw.bat Release

# Debug 构建
build_mingw.bat Debug
```

### 方法 2: 手动 CMake 构建

```batch
# 设置 MinGW 路径
set PATH=C:\mingw64\bin;%PATH%

# 进入项目目录
cd lottie_renderer

# 创建构建目录并配置
cmake -G "MinGW Makefiles" -B build_mingw -DCMAKE_BUILD_TYPE=Release

# 编译 (使用所有 CPU 核心)
cmake --build build_mingw -j%NUMBER_OF_PROCESSORS%
```

### 输出文件

```
build_mingw/liblottie_renderer.a           # 静态库
build_mingw/bin/lottie_test.exe            # C++ 测试程序
build_mingw/bin/lottie_c_test.exe          # C 测试程序
```

---

## Windows - CMake + MSVC

### 命令行构建

```batch
# 进入项目目录
cd lottie_renderer

# 创建构建目录
mkdir build
cd build

# 配置 (Visual Studio 2022, x64)
cmake .. -G "Visual Studio 17 2022" -A x64

# 或 Visual Studio 2019
cmake .. -G "Visual Studio 16 2019" -A x64

# 构建 Release 版本
cmake --build . --config Release

# 构建 Debug 版本
cmake --build . --config Debug
```

### 输出文件

```
build/Release/lottie_renderer.lib          # 静态库
build/bin/Release/lottie_test.exe          # C++ 测试程序
build/bin/Release/lottie_c_test.exe        # C 测试程序
```

---

## Linux

### 安装依赖

```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake

# CentOS/RHEL
sudo yum install gcc gcc-c++ cmake make

# Arch Linux
sudo pacman -S base-devel cmake
```

### 构建步骤

```bash
# 进入项目目录
cd lottie_renderer

# 创建构建目录
mkdir build && cd build

# 配置
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译
make -j$(nproc)

# 或使用 cmake
cmake --build . -j$(nproc)
```

### 输出文件

```
build/liblottie_renderer.a                 # 静态库
build/bin/lottie_test                      # C++ 测试程序
build/bin/lottie_c_test                    # C 测试程序
```

---

## ARM 交叉编译

### 准备工具链文件

创建 `arm-toolchain.cmake`:

```cmake
# ARM 交叉编译工具链配置
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# 设置交叉编译器路径
set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

# 设置查找路径
set(CMAKE_FIND_ROOT_PATH /usr/arm-linux-gnueabihf)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
```

### 构建步骤

```bash
# 进入项目目录
cd lottie_renderer

# 创建构建目录
mkdir build_arm && cd build_arm

# 配置 (使用工具链文件)
cmake .. -DCMAKE_TOOLCHAIN_FILE=../arm-toolchain.cmake \
         -DCMAKE_BUILD_TYPE=Release

# 编译
make -j$(nproc)
```

### AArch64 (ARM64) 配置

```cmake
# AArch64 工具链配置
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
```

---

## 编译选项

### CMake 选项

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `CMAKE_BUILD_TYPE` | Release | 构建类型 (Debug/Release) |
| `LOTTIE_CACHE` | ON | 启用模型缓存 |
| `LOTTIE_THREAD` | OFF | 启用多线程支持 |
| `LOTTIE_MODULE` | OFF | 启用模块加载支持 |
| `LOTTIE_BUILD_TEST` | ON | 构建测试程序 |

### 使用示例

```bash
# 禁用测试程序
cmake .. -DLOTTIE_BUILD_TEST=OFF

# 启用多线程
cmake .. -DLOTTIE_THREAD=ON

# 禁用缓存
cmake .. -DLOTTIE_CACHE=OFF
```

---

## 输出文件

### 文件说明

| 文件 | 平台 | 说明 |
|------|------|------|
| `lottie_renderer.lib` | MSVC | 静态库 |
| `liblottie_renderer.a` | MinGW/GCC | 静态库 |
| `lottie_test.exe` / `lottie_test` | 全平台 | C++ 测试程序 |
| `lottie_c_test.exe` / `lottie_c_test` | 全平台 | C 测试程序 |

### 测试程序使用

```bash
# 播放动画 (Windows)
lottie_c_test animation.json --play

# 保存所有帧为 BMP
lottie_c_test animation.json --save output_dir

# 默认行为 (播放)
lottie_c_test animation.json
```

---

## 常见问题

### Q: MinGW 编译时找不到 gcc

**A:** 确保 MinGW 的 bin 目录在 PATH 中：
```batch
set PATH=C:\mingw64\bin;%PATH%
```

### Q: CMake 找不到编译器

**A:** 指定编译器路径：
```bash
cmake .. -DCMAKE_C_COMPILER=/path/to/gcc \
         -DCMAKE_CXX_COMPILER=/path/to/g++
```

### Q: 链接时报 undefined reference to `std::...`

**A:** 确保链接 C++ 标准库：
```bash
gcc main.c -llottie_renderer -lstdc++ -o app
```

### Q: Windows 下链接失败

**A:** 确保链接必要的系统库：
```
-lshlwapi -lgdi32 -luser32
```

### Q: ARM 编译时 NEON 相关错误

**A:** 检查工具链是否支持 NEON，或禁用 NEON 优化：
```bash
cmake .. -DCMAKE_CXX_FLAGS="-mfpu=vfpv3"
```

### Q: 编译警告 C4819 (MSVC)

**A:** 源文件包含非 ASCII 字符，已在代码中修复。如仍有问题，检查文件编码是否为 UTF-8。

---

## 验证构建

构建完成后，运行测试程序验证：

```bash
# 显示帮助
lottie_c_test

# 测试加载和渲染
lottie_c_test test.json --save output

# 检查输出
ls output/frame_*.bmp
```

如果能正常生成 BMP 文件，说明构建成功。
