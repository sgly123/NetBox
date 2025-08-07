cmake_minimum_required(VERSION 3.16)
project(TestProject)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 项目信息
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

# 编译选项
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -O2)
endif()


# 包含目录
include_directories(include)

# 查找NetBox框架
find_path(NETBOX_INCLUDE_DIR 
    NAMES NetBox.h 
    PATHS ../NetFramework/include
    PATH_SUFFIXES netbox
)

if(NETBOX_INCLUDE_DIR)
    include_directories(${NETBOX_INCLUDE_DIR}/..)
    message(STATUS "Found NetBox framework: ${NETBOX_INCLUDE_DIR}")
else()
    message(WARNING "NetBox framework not found, using local headers")
endif()


# 核心框架库
file(GLOB_RECURSE FRAMEWORK_SOURCES 
    "src/core/*.cpp"
    "src/network/*.cpp" 
    "src/protocol/*.cpp"
    "src/application/*.cpp"
    "src/plugins/*.cpp"
    "src/utils/*.cpp"
)

add_library(TestProject_framework STATIC ${FRAMEWORK_SOURCES})
target_include_directories(TestProject_framework PUBLIC include)

# 特性配置
target_compile_definitions(TestProject_framework PUBLIC NETBOX_ENABLE_LOGGING=1)
target_compile_definitions(TestProject_framework PUBLIC NETBOX_ENABLE_METRICS=1)

# 协议扩展库
file(GLOB_RECURSE PROTOCOL_SOURCES "protocols/*.cpp")
if(PROTOCOL_SOURCES)
    add_library(TestProject_protocols STATIC ${PROTOCOL_SOURCES})
    target_link_libraries(TestProject_protocols TestProject_framework)
    target_include_directories(TestProject_protocols PUBLIC protocols)
endif()

# 应用扩展库
file(GLOB_RECURSE APPLICATION_SOURCES "applications/*.cpp")
if(APPLICATION_SOURCES)
    add_library(TestProject_applications STATIC ${APPLICATION_SOURCES})
    target_link_libraries(TestProject_applications TestProject_framework)
    target_include_directories(TestProject_applications PUBLIC applications)
endif()

# 网络扩展库
file(GLOB_RECURSE NETWORK_SOURCES "network/*.cpp")
if(NETWORK_SOURCES)
    add_library(TestProject_network STATIC ${NETWORK_SOURCES})
    target_link_libraries(TestProject_network TestProject_framework)
    target_include_directories(TestProject_network PUBLIC network)
endif()

# 插件库
file(GLOB_RECURSE PLUGIN_SOURCES "plugins/*.cpp")
if(PLUGIN_SOURCES)
    add_library(TestProject_plugins SHARED ${PLUGIN_SOURCES})
    target_link_libraries(TestProject_plugins TestProject_framework)
    target_include_directories(TestProject_plugins PUBLIC plugins)
    
    # 插件安装
    install(TARGETS TestProject_plugins 
        LIBRARY DESTINATION lib/TestProject/plugins
    )
endif()

# 示例程序
add_subdirectory(examples)

# 测试
enable_testing()
add_subdirectory(tests)


# 安装配置
install(DIRECTORY include/ DESTINATION include)
install(TARGETS TestProject_framework 
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

if(TARGET TestProject_protocols)
    install(TARGETS TestProject_protocols DESTINATION lib)
endif()

if(TARGET TestProject_applications)
    install(TARGETS TestProject_applications DESTINATION lib)
endif()

# 生成配置文件
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/TestProjectConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/TestProjectConfig.cmake"
    @ONLY
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/TestProjectConfig.cmake"
    DESTINATION lib/cmake/TestProject
)

# 生成版本文件
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/TestProjectConfigVersion.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/TestProjectConfigVersion.cmake"
    @ONLY
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/TestProjectConfigVersion.cmake"
    DESTINATION lib/cmake/TestProject
)

# 打包配置
include(CPack)
set(CPACK_PACKAGE_NAME "TestProject")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "TestProject - NetBox Framework Project")
set(CPACK_PACKAGE_VENDOR "NetBox Team")
set(CPACK_GENERATOR "TGZ;ZIP")

# 显示配置信息
message(STATUS "=== TestProject 构建配置 ===")
message(STATUS "项目版本: ${PROJECT_VERSION}")
message(STATUS "C++标准: ${CMAKE_CXX_STANDARD}")
message(STATUS "构建类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "编译器: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "启用特性: logging, metrics")
message(STATUS "安装前缀: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "==================================")