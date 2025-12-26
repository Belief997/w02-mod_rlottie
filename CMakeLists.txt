cmake_minimum_required(VERSION 3.10)

project(lottie_renderer VERSION 1.0 LANGUAGES C CXX)

# C++14 标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# 默认构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 选项配置
option(LOTTIE_CACHE "启用模型缓存支持" ON)
option(LOTTIE_THREAD "启用线程支持" OFF)
option(LOTTIE_MODULE "启用模块加载支持" OFF)
option(LOTTIE_BUILD_TEST "构建测试程序" ON)

# 配置文件生成
if(LOTTIE_CACHE)
    set(LOTTIE_CACHE_SUPPORT 1)
endif()
if(LOTTIE_THREAD)
    set(LOTTIE_THREAD_SUPPORT 1)
endif()
if(LOTTIE_MODULE)
    set(LOTTIE_MODULE_SUPPORT 1)
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

# 创建静态库
add_library(lottie_renderer STATIC)

# 定义 RLOTTIE_BUILD 宏用于内部构建
target_compile_definitions(lottie_renderer PRIVATE RLOTTIE_BUILD)

# 公共头文件目录
target_include_directories(lottie_renderer
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)

# 平台特定编译选项
if(MSVC)
    target_compile_options(lottie_renderer PRIVATE
        /W3
        /wd4244  # 类型转换警告
        /wd4267  # size_t 转换警告
        /wd4996  # 废弃函数警告
    )
    # Windows 特定定义
    target_compile_definitions(lottie_renderer PRIVATE
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
    )
elseif(MINGW)
    # MinGW 特定配置
    target_compile_options(lottie_renderer PRIVATE
        -Wall
        -Wextra
        -Wno-unused-parameter
        -fvisibility=hidden
    )
    target_compile_definitions(lottie_renderer PRIVATE
        _WIN32_WINNT=0x0601
        NOMINMAX
    )
else()
    target_compile_options(lottie_renderer PRIVATE
        -Wall
        -Wextra
        -Wno-unused-parameter
        -fvisibility=hidden
    )
    # ARM NEON 优化
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
        target_compile_options(lottie_renderer PRIVATE -mfpu=neon)
    endif()
endif()

# 线程库
if(LOTTIE_THREAD)
    find_package(Threads REQUIRED)
    target_link_libraries(lottie_renderer PRIVATE Threads::Threads)
endif()

# Windows 特定库
if(WIN32)
    target_link_libraries(lottie_renderer PRIVATE Shlwapi)
endif()

# 添加源文件子目录
add_subdirectory(src)

# 构建测试程序
if(LOTTIE_BUILD_TEST)
    add_subdirectory(test)
endif()

# 安装配置
install(TARGETS lottie_renderer
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(FILES include/lottie_renderer.h
    DESTINATION include
)
