cmake_minimum_required(VERSION 3.10)
project(BinTest VERSION 1.0.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖包


# 设置包含目录
include_directories(
    include
    Protocol/include
    NetFramework/include
)

# 检查框架代码是否存在
if(EXISTS "Protocol/include/ProtocolBase.h")
    message(STATUS "Found NetBox Protocol framework - building with framework support")
    
    # 收集框架源文件
    file(GLOB_RECURSE PROTOCOL_SOURCES "Protocol/src/*.cpp")
    file(GLOB_RECURSE NETFRAMEWORK_SOURCES "NetFramework/src/*.cpp")
    
    # 创建框架静态库
    add_library(netbox_framework STATIC
        ${PROTOCOL_SOURCES}
        ${NETFRAMEWORK_SOURCES}
    )
    
    # 设置框架库的包含目录
    target_include_directories(netbox_framework PUBLIC
        include
        Protocol/include
        NetFramework/include
    )
    
    # 链接框架库的依赖
    
    
    # 添加主程序
    add_executable(BinTest src/main.cpp)
    
    # 设置主程序的包含目录
    target_include_directories(BinTest PRIVATE 
        include
        Protocol/include
        NetFramework/include
    )
    
    # 链接框架库
    target_link_libraries(BinTest PRIVATE netbox_framework)
    
    # 添加测试
    enable_testing()
    add_executable(BinTest_test tests/simple_test.cpp)
    
    # 设置测试程序的包含目录
    target_include_directories(BinTest_test PRIVATE 
        include
        Protocol/include
        NetFramework/include
    )
    
    # 链接测试程序到框架库
    target_link_libraries(BinTest_test PRIVATE netbox_framework)
    
    # 添加示例
    add_executable(BinTest_example examples/basic/simple_example.cpp)
    
    # 设置示例程序的包含目录
    target_include_directories(BinTest_example PRIVATE 
        include
        Protocol/include
        NetFramework/include
    )
    
    # 链接示例程序到框架库
    target_link_libraries(BinTest_example PRIVATE netbox_framework)
    
    # 创建自定义协议开发示例
    add_executable(BinTest_custom_protocol examples/custom_protocol_example.cpp)
    target_include_directories(BinTest_custom_protocol PRIVATE 
        include
        Protocol/include
        NetFramework/include
    )
    target_link_libraries(BinTest_custom_protocol PRIVATE netbox_framework)
    
    # 设置编译选项
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(BinTest PRIVATE -g -O0 -Wall -Wextra)
        target_compile_options(BinTest_test PRIVATE -g -O0 -Wall -Wextra)
        target_compile_options(BinTest_example PRIVATE -g -O0 -Wall -Wextra)
        target_compile_options(BinTest_custom_protocol PRIVATE -g -O0 -Wall -Wextra)
        target_compile_options(netbox_framework PRIVATE -g -O0 -Wall -Wextra)
    else()
        target_compile_options(BinTest PRIVATE -O2 -Wall -Wextra)
        target_compile_options(BinTest_test PRIVATE -O2 -Wall -Wextra)
        target_compile_options(BinTest_example PRIVATE -O2 -Wall -Wextra)
        target_compile_options(BinTest_custom_protocol PRIVATE -O2 -Wall -Wextra)
        target_compile_options(netbox_framework PRIVATE -O2 -Wall -Wextra)
    endif()
    
else()
    message(STATUS "NetBox framework not found - using basic mode")
    
    # 基本模式：只编译主程序
    add_executable(BinTest src/main.cpp)
    target_include_directories(BinTest PRIVATE include)
    
    # 添加测试
    enable_testing()
    add_executable(BinTest_test tests/simple_test.cpp)
    target_include_directories(BinTest_test PRIVATE include)
    
    # 添加示例
    add_executable(BinTest_example examples/basic/simple_example.cpp)
    target_include_directories(BinTest_example PRIVATE include)
    
    # 设置编译选项
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(BinTest PRIVATE -g -O0 -Wall -Wextra)
        target_compile_options(BinTest_test PRIVATE -g -O0 -Wall -Wextra)
        target_compile_options(BinTest_example PRIVATE -g -O0 -Wall -Wextra)
    else()
        target_compile_options(BinTest PRIVATE -O2 -Wall -Wextra)
        target_compile_options(BinTest_test PRIVATE -O2 -Wall -Wextra)
        target_compile_options(BinTest_example PRIVATE -O2 -Wall -Wextra)
    endif()
endif()

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
