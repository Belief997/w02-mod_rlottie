# Lottie Renderer 移植指南

本文档说明如何将 `liblottie_renderer.a` 静态库移植到其他项目中使用。

## 文件清单

移植时需要以下文件：

```
liblottie_renderer.a      # 静态库文件
lottie_renderer.h         # C API 头文件
```

## 移植步骤

### 1. 复制文件

将以下文件复制到目标项目：

```bash
# 头文件
cp lottie_renderer/include/lottie_renderer.h  <your_project>/include/

# 静态库 (MinGW 构建)
cp lottie_renderer/build_mingw/liblottie_renderer.a  <your_project>/lib/

# 或 MSVC 构建
cp lottie_renderer/vs2019/x64/Release/lottie_renderer.lib  <your_project>/lib/
```

### 2. 编译器配置

#### MinGW / GCC

```makefile
# Makefile 示例
CC = gcc
CXX = g++
CFLAGS = -I./include
LDFLAGS = -L./lib -llottie_renderer -lstdc++ -lshlwapi

# Windows GDI 支持 (可选，用于播放功能)
LDFLAGS += -lgdi32 -luser32

app: main.c
    $(CC) $(CFLAGS) main.c -o app $(LDFLAGS)
```

#### CMake

```cmake
# CMakeLists.txt 示例
cmake_minimum_required(VERSION 3.10)
project(MyApp)

# 添加头文件路径
include_directories(${CMAKE_SOURCE_DIR}/include)

# 添加库文件路径
link_directories(${CMAKE_SOURCE_DIR}/lib)

# 创建可执行文件
add_executable(myapp main.c)

# 链接库
target_link_libraries(myapp
    lottie_renderer
    stdc++
    shlwapi
)

# Windows 平台额外链接
if(WIN32)
    target_link_libraries(myapp gdi32 user32)
endif()
```

### 3. 代码示例

#### 基本使用 (C 语言)

```c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "lottie_renderer.h"

int main(int argc, char* argv[]) {
    // 1. 加载动画
    LottieAnimationHandle anim = lottie_animation_from_file("animation.json");
    if (!anim) {
        fprintf(stderr, "Failed to load animation\n");
        return 1;
    }
    
    // 2. 获取动画信息
    LottieAnimationInfo info;
    lottie_animation_get_info(anim, &info);
    printf("Frame Rate: %.2f fps\n", info.frameRate);
    printf("Total Frames: %zu\n", info.totalFrames);
    printf("Size: %zu x %zu\n", info.width, info.height);
    
    // 3. 分配渲染缓冲区
    size_t width = info.width;
    size_t height = info.height;
    uint32_t* buffer = (uint32_t*)malloc(width * height * sizeof(uint32_t));
    
    // 4. 创建渲染表面
    LottieSurface surface;
    surface.buffer = buffer;
    surface.width = width;
    surface.height = height;
    surface.bytesPerLine = width * sizeof(uint32_t);
    
    // 5. 渲染指定帧
    size_t frameNo = 0;
    memset(buffer, 0, width * height * sizeof(uint32_t));
    lottie_animation_render(anim, frameNo, &surface, 1);
    
    // 6. 使用渲染结果 (buffer 包含 ARGB32 预乘像素数据)
    // ... 显示或保存图像 ...
    
    // 7. 清理资源
    free(buffer);
    lottie_animation_destroy(anim);
    
    return 0;
}
```

#### 从内存加载

```c
// 从 JSON 字符串加载
const char* jsonData = "{ ... }";  // Lottie JSON 内容
size_t dataSize = strlen(jsonData);

LottieAnimationHandle anim = lottie_animation_from_data(
    jsonData, 
    dataSize, 
    NULL  // 资源路径，用于外部图片
);
```

#### 动画循环播放

```c
// 计算帧间隔
double frameInterval = 1000.0 / info.frameRate;  // 毫秒

// 播放循环
size_t currentFrame = 0;
while (playing) {
    // 渲染当前帧
    memset(buffer, 0, width * height * sizeof(uint32_t));
    lottie_animation_render(anim, currentFrame, &surface, 1);
    
    // 显示帧
    display_frame(buffer, width, height);
    
    // 下一帧
    currentFrame = (currentFrame + 1) % info.totalFrames;
    
    // 等待帧间隔
    sleep_ms(frameInterval);
}
```

### 4. 像素格式说明

渲染输出使用 **ARGB32 预乘格式**：

- 每像素 4 字节 (32 位)
- 内存布局 (小端序): `[B, G, R, A]`
- 位布局: `0xAARRGGBB`
- Alpha 预乘: RGB 值已乘以 Alpha

#### 使用内置转换函数 (推荐)

```c
// 渲染帧
lottie_animation_render(anim, frameNo, &surface, 1);

// 转换为非预乘格式 (straight alpha)
lottie_convert_to_straight_alpha(buffer, width, height);

// 或转换为 RGBA 格式 (用于 OpenGL/PNG)
lottie_convert_argb_to_rgba(buffer, width, height);

// 或一步完成: 非预乘 + RGBA
lottie_convert_to_straight_rgba(buffer, width, height);
```

#### 手动转换为标准 RGBA

```c
void premultiplied_to_straight(uint32_t* buffer, size_t count) {
    for (size_t i = 0; i < count; i++) {
        uint32_t pixel = buffer[i];
        uint8_t a = (pixel >> 24) & 0xFF;
        if (a > 0 && a < 255) {
            uint8_t r = ((pixel >> 16) & 0xFF) * 255 / a;
            uint8_t g = ((pixel >> 8) & 0xFF) * 255 / a;
            uint8_t b = (pixel & 0xFF) * 255 / a;
            buffer[i] = (a << 24) | (r << 16) | (g << 8) | b;
        }
    }
}
```

#### 与棋盘格背景混合 (显示透明度)

```c
uint32_t blend_with_checker(uint32_t pixel, int x, int y) {
    uint8_t a = (pixel >> 24) & 0xFF;
    
    // 棋盘格颜色
    int cell = 8;  // 格子大小
    uint32_t bg = ((x/cell + y/cell) % 2 == 0) ? 0xFFCCCCCC : 0xFF999999;
    
    if (a == 255) return pixel;
    if (a == 0) return bg;
    
    // 预乘混合: out = src + bg * (1 - srcA)
    uint8_t srcR = (pixel >> 16) & 0xFF;
    uint8_t srcG = (pixel >> 8) & 0xFF;
    uint8_t srcB = pixel & 0xFF;
    uint8_t bgR = (bg >> 16) & 0xFF;
    uint8_t bgG = (bg >> 8) & 0xFF;
    uint8_t bgB = bg & 0xFF;
    uint8_t invA = 255 - a;
    
    return 0xFF000000 |
           ((srcR + bgR * invA / 255) << 16) |
           ((srcG + bgG * invA / 255) << 8) |
           (srcB + bgB * invA / 255);
}
```

### 5. API 参考

| 函数 | 说明 |
|------|------|
| `lottie_animation_from_file()` | 从文件加载动画 |
| `lottie_animation_from_data()` | 从 JSON 字符串加载 |
| `lottie_animation_get_info()` | 获取动画信息 |
| `lottie_animation_get_framerate()` | 获取帧率 |
| `lottie_animation_get_totalframe()` | 获取总帧数 |
| `lottie_animation_get_duration()` | 获取时长 (秒) |
| `lottie_animation_get_size()` | 获取原始尺寸 |
| `lottie_animation_render()` | 渲染指定帧 |
| `lottie_animation_frame_at_pos()` | 根据位置获取帧号 |
| `lottie_animation_to_json()` | 导出动画信息为 JSON |
| `lottie_free_string()` | 释放字符串 |
| `lottie_animation_destroy()` | 释放动画资源 |
| `lottie_configure_cache_size()` | 配置缓存大小 |
| `lottie_convert_to_straight_alpha()` | 预乘转非预乘 (ARGB) |
| `lottie_convert_argb_to_rgba()` | ARGB 转 RGBA |
| `lottie_convert_to_straight_rgba()` | 预乘 ARGB 转非预乘 RGBA |

### 6. 平台注意事项

#### Windows (MinGW)
- 需要链接 `stdc++` (C++ 标准库)
- 需要链接 `shlwapi` (路径处理)
- GDI 播放需要 `gdi32`, `user32`

#### Windows (MSVC)
- 使用 `.lib` 文件而非 `.a`
- 自动链接 C++ 运行时

#### Linux / ARM
- 需要链接 `stdc++`, `pthread`
- ARM 平台自动启用 NEON 优化

### 7. 常见问题

**Q: 链接时报 undefined reference to `std::...`**
A: 需要链接 C++ 标准库 `-lstdc++`

**Q: 渲染结果颜色不对**
A: 检查像素格式，输出是 ARGB32 预乘格式

**Q: 动画播放速度不对**
A: 使用 `info.frameRate` 计算正确的帧间隔

**Q: 内存泄漏**
A: 确保调用 `lottie_animation_destroy()` 释放资源
