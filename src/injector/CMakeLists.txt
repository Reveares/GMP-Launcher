cmake_minimum_required(VERSION 3.21)

add_executable(gmpinjector main.cpp)

set_target_properties(gmpinjector PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
target_compile_definitions(gmpinjector PRIVATE
        WIN32_LEAN_AND_MEAN
        UNICODE
        _UNICODE
        _CRT_SECURE_NO_WARNINGS
        _HAS_EXCEPTIONS=0
)
target_compile_options(gmpinjector PRIVATE
        /W4
        /WX
        /permissive-
        /EHs-c-
        /utf-8
        /GR-
)
