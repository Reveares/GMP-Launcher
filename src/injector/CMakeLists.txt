cmake_minimum_required(VERSION 3.22)

add_executable(gmpinjector main.cpp)

# The top-level C++20 setting does not reach a standalone configure of this directory.
target_compile_features(gmpinjector PRIVATE cxx_std_20)

target_compile_definitions(gmpinjector PRIVATE
        WIN32_LEAN_AND_MEAN
        UNICODE
        _UNICODE
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

install(TARGETS gmpinjector)
