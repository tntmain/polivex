cmake_minimum_required(VERSION 3.26)

project(Polivex
    VERSION 0.1.0
    DESCRIPTION "Open-source CAD application for Linux and Windows"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(POLIVEX_BUILD_TESTS "Build Polivex tests" ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets)

add_subdirectory(src)

if(POLIVEX_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
