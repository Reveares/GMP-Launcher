cmake_minimum_required(VERSION 3.22)

find_package(Git REQUIRED)
find_package(Qt6 CONFIG REQUIRED COMPONENTS Widgets)

set(SLIKENET_ENABLE_SAMPLES OFF CACHE INTERNAL "")
set(SLIKENET_ENABLE_DLL OFF CACHE INTERNAL "")

include(FetchContent)
FetchContent_Declare(slikenet
        GIT_REPOSITORY https://gitlab.com/Sabrosa/slikenet.git
        GIT_TAG        0621404add46ac03decb1b46fed8fd79e982abbc
)
FetchContent_MakeAvailable(slikenet)

qt_standard_project_setup()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
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
        WIN32_EXECUTABLE ON # Prevent the creation of a console window on Windows
        MACOSX_BUNDLE ON # Create an application bundle on macOS.
)

target_link_libraries(gml PRIVATE
        Qt6::Widgets
        SLikeNetLibStatic
)

target_include_directories(gml PRIVATE
        "${slikenet_SOURCE_DIR}/Source/include"
        "${PROJECT_BINARY_DIR}/include/launcher"
)

if (MSVC)
    target_compile_definitions(gml PRIVATE WIN32_LEAN_AND_MEAN)
    target_compile_options(gml PRIVATE
            /W4
            /WX
            /permissive-
            /EHsc
            /utf-8
    )
else ()
    target_compile_options(gml PRIVATE
            -Wall
            -Wextra
            -Wshadow
            -Wnon-virtual-dtor
            -pedantic
    )

    add_custom_command(TARGET gml POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_SOURCE_DIR}/resource/gmpinjector.sh" $<TARGET_FILE_DIR:gml>
    )
endif ()

install(TARGETS gml
        BUNDLE  DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

set(QT_DEPLOY_PLUGINS_DIR .)
qt_generate_deploy_app_script(
        TARGET gml
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
        NO_TRANSLATIONS
        NO_COMPILER_RUNTIME
)
install(SCRIPT ${deploy_script})
