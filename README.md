# Lottie Renderer 模块

从 rlottie 提取的独立 Lottie 动画渲染模块，支持：
- 解析 Lottie JSON 文件
- 渲染指定帧为 ARGB32 像素数据
- 跨平台支持 (Windows MSVC/MinGW, ARM)

## 目录结构

```
lottie_renderer/
├── CMakeLists.txt          # CMake 主构建配置
├── config.h.in             # 配置模板
├── build_mingw.bat         # MinGW 构建脚本
├── include/
│   └── lottie_renderer.h   # 公共 C API 头文件
├── src/
│   ├── lottie/             # Lottie 解析和动画逻辑
│   ├── vector/             # 矢量图形渲染引擎
│   └── lottie_renderer_api.cpp  # C API 实现
├── test/
│   ├── win/                # C++ 测试程序
│   └── c_test/             # C 测试程序
├── vs2019/                 # Visual Studio 解决方案
└── docs/
    ├── BUILD.md            # 编译指南
    └── PORTING.md          # 移植指南
```

## 构建方法

### Windows - Visual Studio (推荐)

直接打开解决方案文件：
```
lottie_renderer/vs2019/lottie_renderer.sln
```

### Windows - MinGW

```batch
# 方法1: 使用构建脚本
build_mingw.bat Release

# 方法2: 手动 CMake
set PATH=C:\mingw64\bin;%PATH%
cmake -G "MinGW Makefiles" -B build_mingw -DCMAKE_BUILD_TYPE=Release
cmake --build build_mingw
```

输出文件：
- 库文件: `build_mingw/liblottie_renderer.a`
- C 测试: `build_mingw/bin/lottie_c_test.exe`
- C++ 测试: `build_mingw/bin/lottie_test.exe`

### Windows - CMake + MSVC

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### ARM 交叉编译

```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=<arm-toolchain.cmake>
cmake --build .
```

## 文档

- [编译指南](docs/BUILD.md) - 详细的编译步骤说明
- [移植指南](docs/PORTING.md) - 如何移植到其他项目

## 移植到其他项目

详见 [移植指南](docs/PORTING.md)

快速开始：
```bash
# 复制文件
cp include/lottie_renderer.h  <your_project>/include/
cp build_mingw/liblottie_renderer.a  <your_project>/lib/

# 编译链接
gcc -I./include main.c -L./lib -llottie_renderer -lstdc++ -lshlwapi -o app
```

## 使用方法

### C API

```c
#include "lottie_renderer.h"

// 加载动画
LottieAnimationHandle anim = lottie_animation_from_file("animation.json");

// 获取信息
LottieAnimationInfo info;
lottie_animation_get_info(anim, &info);
printf("帧率: %.2f, 总帧数: %zu\n", info.frameRate, info.totalFrames);

// 渲染帧
uint32_t* buffer = malloc(width * height * 4);
LottieSurface surface = {buffer, width, height, width * 4};
lottie_animation_render(anim, 0, &surface, 1);

// 释放资源
lottie_animation_destroy(anim);
free(buffer);
```

### 测试程序

```bash
# 渲染所有帧为 BMP 文件
lottie_test animation.json output_dir

# 播放动画 (仅 Windows)
lottie_test animation.json --play
```

## API 参考

### 加载函数
- `lottie_animation_from_file()` - 从文件加载
- `lottie_animation_from_data()` - 从 JSON 字符串加载

### 信息查询
- `lottie_animation_get_info()` - 获取完整动画信息
- `lottie_animation_get_framerate()` - 获取帧率
- `lottie_animation_get_totalframe()` - 获取总帧数
- `lottie_animation_get_duration()` - 获取时长
- `lottie_animation_get_size()` - 获取尺寸

### 渲染函数
- `lottie_animation_render()` - 同步渲染指定帧
- `lottie_animation_frame_at_pos()` - 根据位置获取帧号

### 资源管理
- `lottie_animation_destroy()` - 释放动画资源
- `lottie_configure_cache_size()` - 配置缓存大小

## 像素格式

渲染输出使用 ARGB32 预乘格式：
- 每像素 4 字节
- 字节顺序：B, G, R, A（小端序）
- Alpha 预乘：RGB 值已乘以 Alpha

## 许可证

基于 rlottie，采用 MIT 许可证。
