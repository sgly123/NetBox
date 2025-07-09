cmake_minimum_required(VERSION 3.29)
project(
    Base_tool
    VERSION 0.1
    LANGUAGES CXX
    )
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(
    GLOB SRC_FILES 
    "src/*.cpp"
    )
file(
    GLOB INC_FILES 
    "include/*.h"
    )
    add_library( Base_tool STATIC
    ${SRC_FILES}
    ${INC_FILES}
)