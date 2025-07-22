cmake_minimum_required(VERSION 3.29)
project(NetBox)

# 收集源文件和头文件
file(GLOB_RECURSE 
SOURCES 
"NetFramework/src/*.cpp" 
"main.cpp"
)
file(GLOB_RECURSE 
HEADERS 
"NetFramework/include/*.h"
)

include_directories(NetFramework/include)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加可执行文件
add_executable(NetBox ${SOURCES} ${HEADERS})


target_include_directories(NetBox PRIVATE include)
target_link_libraries(NetBox pthread)