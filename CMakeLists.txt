cmake_minimum_required(VERSION 3.16)

project(GothicMultiplayerLauncher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SLIKENET_ENABLE_SAMPLES OFF CACHE INTERNAL "")
set(SLIKENET_ENABLE_DLL OFF CACHE INTERNAL "")
add_subdirectory(libs/slikenet)

find_package(Git REQUIRED)
find_package(Qt6 CONFIG REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()
set(CMAKE_AUTORCC ON)

execute_process(COMMAND
        "${GIT_EXECUTABLE}" describe --tags --abbrev=0
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_TAG
        ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(COMMAND
        "${GIT_EXECUTABLE}" log -1 --pretty=format:%h --no-show-signature
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_COMMIT
        ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(COMMAND
        "${GIT_EXECUTABLE}" log -1 --date=format:%Y-%m-%d\ %H:%M:%S --pretty=format:%ad --no-show-signature
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_DATE
        ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/resource.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/src/resource.h" @ONLY)

qt_add_executable(GothicMultiplayerLauncher
        src/dialogaddserver.cpp
        src/dialogaddserver.ui
        src/dialoginfo.cpp
        src/dialoginfo.ui
        src/gmpclient.cpp
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.ui
        src/options.cpp
        src/options.ui
        src/server.cpp
        src/servermodel.cpp
        resource/resource.qrc
        resource/resource.rc
)

set_target_properties(GothicMultiplayerLauncher PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        OUTPUT_NAME "gml"
        WIN32_EXECUTABLE ON # Prevent the creation of a console window on Windows
        MACOSX_BUNDLE ON # Create an application bundle on macOS.
)

target_link_libraries(GothicMultiplayerLauncher PRIVATE
        Qt6::Widgets
        SLikeNetLibStatic
)

target_include_directories(GothicMultiplayerLauncher PRIVATE
        libs/slikenet/Source/include
)

if (MSVC)
    target_compile_definitions(GothicMultiplayerLauncher PRIVATE
            WIN32_LEAN_AND_MEAN
            UNICODE
            _UNICODE
            _CRT_SECURE_NO_WARNINGS
    )
    target_compile_options(GothicMultiplayerLauncher PRIVATE
            /W4
            /WX
            /permissive-
            /EHsc
            /utf-8
    )

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        find_program(TOOL_WINDEPLOYQT NAMES windeployqt.debug.bat)
    else ()
        find_program(TOOL_WINDEPLOYQT NAMES windeployqt)
    endif ()

    add_custom_command(TARGET GothicMultiplayerLauncher POST_BUILD
            COMMAND ${TOOL_WINDEPLOYQT} --no-compiler-runtime
            $<TARGET_FILE:GothicMultiplayerLauncher>
            COMMENT "Running windeployqt..."
    )
else ()
    target_compile_options(GothicMultiplayerLauncher PRIVATE
            -fpermissive
            -static-libstdc++
            -static-libgcc
            -Wall
            -Wextra
            -O3
            -s
            -Wclobbered
            -Wignored-qualifiers
            -Wuninitialized
            -Wold-style-cast
            -Wreorder
            -Wno-unused-parameter
            -Wno-unknown-pragmas
            -Wno-unused-function
            -fno-rtti
            -fno-jump-tables
            -ffunction-sections
            -fdata-sections
            -fstack-protector-strong
            -Wl,--gc-sections
            -Wl,--strip-all
    )
endif ()
