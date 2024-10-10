cmake_minimum_required(VERSION 3.22)

project(GothicMultiplayerLauncher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(src/launcher)

if (MSVC)
    add_subdirectory(src/injector)
    add_dependencies(gml gmpinjector)
endif ()
