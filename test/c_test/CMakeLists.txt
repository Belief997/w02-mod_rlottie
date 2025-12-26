# C API Test Program

add_executable(lottie_c_test
    c_test.c
)

# Link lottie_renderer library
target_link_libraries(lottie_c_test PRIVATE
    lottie_renderer
)

# Include public headers
target_include_directories(lottie_c_test PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Force C compilation
set_target_properties(lottie_c_test PROPERTIES
    LINKER_LANGUAGE C
)

# Output directory
set_target_properties(lottie_c_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
