# Lottie Renderer 模块

从 rlottie 提取的独立 Lottie 动画渲染模块，支持：
- 解析 Lottie JSON 文件
- 渲染指定帧为 ARGB32 像素数据
- 跨平台支持 (Windows, ARM)

## 目录结构

```
lottie_renderer/
├── CMakeLists.txt          # CMake 主构建配置
├── config.h.in             # 配置模板
├── include/
│   └── lottie_renderer.h   # 公共 API 头文件
├── src/
│   ├── lottie/             # Lottie 解析和动画逻辑
│   ├── vector/             # 矢量图形渲染引擎
│   └── lottie_renderer_api.cpp  # C API 实现
├── test/
│   └── win/
│       ├── win_test.cpp    # Windows 测试程序
│       └── bmp_writer.h    # BMP 文件写入工具
└── vs2019/
    ├── lottie_renderer.sln     # VS 解决方案
    ├── lottie_renderer.vcxproj # 静态库项目
    ├── lottie_test.vcxproj     # 测试程序项目
    └── config.h                # VS 配置文件
```

## 构建方法

### Windows - Visual Studio 解决方案（推荐）

直接使用 Visual Studio 打开解决方案文件：

```
lottie_renderer/vs2019/lottie_renderer.sln
```

支持 Debug/Release 和 Win32/x64 配置。

### Windows - CMake

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### ARM 交叉编译

```bash
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=<arm-toolchain.cmake>
cmake --build .
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
