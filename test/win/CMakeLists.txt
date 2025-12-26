# Windows Test Program

add_executable(lottie_test
    win_test.cpp
)

# Link lottie_renderer library
target_link_libraries(lottie_test PRIVATE
    lottie_renderer
)

# Include public headers
target_include_directories(lottie_test PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Windows specific libraries (for GDI playback)
if(WIN32)
    target_link_libraries(lottie_test PRIVATE
        gdi32
        user32
    )
    
    # Console application
    set_target_properties(lottie_test PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

# Output directory
set_target_properties(lottie_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
