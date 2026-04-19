cmake_minimum_required(VERSION 3.10)
project(SolarSym)

set(CMAKE_CXX_STANDARD 17)

if(WIN32)
    set(OS_WINDOWS TRUE)
    message(STATUS "Building for Windows")
elseif(UNIX)
    set(OS_LINUX TRUE)
    message(STATUS "Building for Linux")
endif()

if(WIN32)
    if(DEFINED ENV{SFML_DIR})
        set(SFML_DIR $ENV{SFML_DIR})
    endif()
    if(DEFINED ENV{NLOHMANN_JSON_DIR})
        set(nlohmann_json_DIR $ENV{NLOHMANN_JSON_DIR})
    endif()

    add_definitions(-DSFML_STATIC)

    set(EXTRA_LIBS
        opengl32
        freetype
        winmm
        gdi32
        ws2_32
        sfml-main
    )
else()
    set(EXTRA_LIBS "")
endif()

find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(solar_sym
    src/main.cpp
    src/utils.cpp
    src/object.cpp
    src/rendering.cpp
    src/scene.cpp
    src/load.cpp
    src/ui.cpp
)

target_include_directories(solar_sym PRIVATE include)

if(WIN32)
    target_link_libraries(solar_sym
        PRIVATE
        sfml-graphics-s
        sfml-window-s
        sfml-system-s
        nlohmann_json::nlohmann_json
        ${EXTRA_LIBS}
    )
else()
    target_link_libraries(solar_sym
        PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
        nlohmann_json::nlohmann_json
        ${EXTRA_LIBS}
    )
endif()