cmake_minimum_required(VERSION 3.22)

project(GothicMultiplayerLauncher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INSTALL_BINDIR . CACHE PATH "Installation directory for executables")
include(GNUInstallDirs)

add_subdirectory(src/launcher)

if (MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    add_subdirectory(src/injector)
    add_dependencies(gml gmpinjector)
endif ()
