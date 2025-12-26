# Windows 测试程序构建配置

add_executable(lottie_test
    win_test.cpp
)

# 链接 lottie_renderer 库
target_link_libraries(lottie_test PRIVATE
    lottie_renderer
)

# 包含公共头文件
target_include_directories(lottie_test PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Windows 特定配置
if(WIN32)
    # 链接 GDI 库
    target_link_libraries(lottie_test PRIVATE
        gdi32
        user32
    )
    
    # 设置为控制台应用程序
    set_target_properties(lottie_test PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

# 设置输出目录
set_target_properties(lottie_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
