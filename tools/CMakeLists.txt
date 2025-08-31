# NetBox压力测试工具CMakeLists.txt

cmake_minimum_required(VERSION 3.16)
project(NetBox_StressTest)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 编译选项
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")

# 如果没有指定构建类型，默认为Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/../app/include
    ${CMAKE_SOURCE_DIR}/../NetFramework/include
    ${CMAKE_SOURCE_DIR}/../Protocol/include
)

# 链接目录
link_directories(
    ${CMAKE_SOURCE_DIR}/../build/NetFramework
    ${CMAKE_SOURCE_DIR}/../build/Protocol
    ${CMAKE_SOURCE_DIR}/../build/app
)

# 查找线程库
find_package(Threads REQUIRED)

# 编译压力测试服务器
add_executable(stress_test_echo
    stress_test_echo.cpp
)

target_link_libraries(stress_test_echo
    NetFramework
    Protocol
    Threads::Threads
)

# 编译压力测试客户端
add_executable(benchmark_client
    benchmark_client.cpp
)

target_link_libraries(benchmark_client
    Protocol
    Threads::Threads
)

# 设置输出目录
set_target_properties(stress_test_echo benchmark_client
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# 安装目标（可选）
install(TARGETS stress_test_echo benchmark_client
    RUNTIME DESTINATION bin
) 