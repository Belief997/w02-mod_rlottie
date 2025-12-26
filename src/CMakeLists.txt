# Lottie Renderer 源文件构建配置

# Vector 图形引擎源文件
set(VECTOR_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/vector/vrect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector/vdasher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector/vbrush.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector/vbitmap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector/vpainter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector/vdrawhelper_common.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector/vdrawhelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector/vrle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector/vpath.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector/vpathmesure.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector/vmatrix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector/velapsedtimer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector/vdebug.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector/vinterpolator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector/vbezier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector/vraster.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector/vdrawable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector/vimageloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector/varenaalloc.cpp
)

# 平台特定的 SIMD 优化
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86|x86_64|AMD64")
    list(APPEND VECTOR_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/vector/vdrawhelper_sse2.cpp
    )
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64|ARM")
    list(APPEND VECTOR_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/vector/vdrawhelper_neon.cpp
    )
endif()

# FreeType 光栅化源文件
set(FREETYPE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/vector/freetype/v_ft_math.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector/freetype/v_ft_raster.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector/freetype/v_ft_stroker.cpp
)

# STB 图片加载源文件
set(STB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/vector/stb/stb_image.cpp
)

# Lottie 解析和动画源文件
set(LOTTIE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/lottie/lottieanimation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lottie/lottieitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lottie/lottieitem_capi.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lottie/lottieloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lottie/lottiemodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lottie/lottieparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lottie/lottieproxymodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lottie/lottiekeypath.cpp
)

# ZIP 解压源文件
set(ZIP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/lottie/zip/zip.cpp
)

# C API 包装层
set(API_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/lottie_renderer_api.cpp
)

# 添加所有源文件到库
target_sources(lottie_renderer PRIVATE
    ${VECTOR_SOURCES}
    ${FREETYPE_SOURCES}
    ${STB_SOURCES}
    ${LOTTIE_SOURCES}
    ${ZIP_SOURCES}
    ${API_SOURCES}
)

# 内部头文件包含路径
target_include_directories(lottie_renderer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/vector
    ${CMAKE_CURRENT_SOURCE_DIR}/vector/freetype
    ${CMAKE_CURRENT_SOURCE_DIR}/vector/stb
    ${CMAKE_CURRENT_SOURCE_DIR}/lottie
    ${CMAKE_CURRENT_SOURCE_DIR}/lottie/rapidjson
)

# 复制原始 rlottie 头文件作为内部依赖
# 这些头文件被 lottie_renderer_api.cpp 使用
