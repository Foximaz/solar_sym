cmake_minimum_required(VERSION 3.10)
project(SolarSym)

set(CMAKE_CXX_STANDARD 17)

find_package(SFML COMPONENTS graphics window system REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(solar_sym
    src/main.cpp
    src/utils.cpp
    src/object.cpp
    src/rendering.cpp
    src/scene.cpp
    src/load.cpp
)

target_include_directories(solar_sym PRIVATE include)

target_link_libraries(solar_sym
    PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
    nlohmann_json::nlohmann_json
)