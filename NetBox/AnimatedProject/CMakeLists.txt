cmake_minimum_required(VERSION 3.10)
project(AnimatedProject VERSION 1.0.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖包
find_package(openssl REQUIRED)
find_package(zlib REQUIRED)

# 添加可执行文件
add_executable(AnimatedProject src/main.cpp)

# 设置包含目录
target_include_directories(AnimatedProject PRIVATE include)

# 链接库
target_link_libraries(AnimatedProject PRIVATE netbox)

# 添加测试
enable_testing()
add_executable(AnimatedProject_test tests/simple_test.cpp)
target_link_libraries(AnimatedProject_test PRIVATE netbox)

# 添加示例
add_executable(AnimatedProject_example examples/basic/simple_example.cpp)
target_link_libraries(AnimatedProject_example PRIVATE netbox)

# 设置输出目录
set_target_properties(AnimatedProject PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(AnimatedProject_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(AnimatedProject_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
