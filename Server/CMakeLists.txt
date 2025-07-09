cmake_minimum_required(VERSION 3.29)
project(
    Scaffold
    VERSION 0.1
    LANGUAGES CXX
    )
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(MSVC)
    add_compile_options(/W4 /WX)  # MSVC 严格模式（/WX 将警告视为错误）
else()
    add_compile_options(-Wall -Wextra -Wpedantic)  # GCC/Clang 警告
endif()
set(CMAKE_BUILD_TYPE Debug)
file(
    GLOB
    SOURCES
    "src/*.cpp"
    )
file(
    GLOB
    HEADERS 
    "include/*.h"
    )
add_executable(
    Scaffold
    ${SOURCES}
    ${HEADERS}
    )

add_subdirectory(../proto Proto)
target_link_libraries(Scaffold PRIVATE protocol_Msg)

add_subdirectory(../base_tool Base_tool)
target_link_libraries(Scaffold PRIVATE Base_tool)
