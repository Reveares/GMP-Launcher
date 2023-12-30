cmake_minimum_required(VERSION 3.21)

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

configure_file("resource.h.in" "${PROJECT_BINARY_DIR}/include/launcher/resource.h" @ONLY)

qt_add_executable(gml
        dialogaddserver.cpp
        dialogaddserver.ui
        dialoginfo.cpp
        dialoginfo.ui
        gmpclient.cpp
        main.cpp
        mainwindow.cpp
        mainwindow.ui
        options.cpp
        options.ui
        server.cpp
        servermodel.cpp
        ../../resource/resource.qrc
        ../../resource/resource.rc
)

set_target_properties(gml PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        WIN32_EXECUTABLE ON # Prevent the creation of a console window on Windows
        MACOSX_BUNDLE ON # Create an application bundle on macOS.
)

target_link_libraries(gml PRIVATE
        Qt6::Widgets
        SLikeNetLibStatic
)

target_include_directories(gml PRIVATE
        ../../libs/slikenet/Source/include
        ${PROJECT_BINARY_DIR}/include/launcher
)

if (MSVC)
    target_compile_definitions(gml PRIVATE
            WIN32_LEAN_AND_MEAN
            UNICODE
            _UNICODE
            _CRT_SECURE_NO_WARNINGS
    )
    target_compile_options(gml PRIVATE
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

    add_custom_command(TARGET gml POST_BUILD
            COMMAND ${TOOL_WINDEPLOYQT} --no-compiler-runtime --no-translations
            $<TARGET_FILE:gml>
            COMMENT "Running windeployqt..."
    )
else ()
    target_compile_options(gml PRIVATE
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
