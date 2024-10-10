cmake_minimum_required(VERSION 3.22)

add_executable(gmpinjector main.cpp)

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

install(TARGETS gmpinjector
        BUNDLE  DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
