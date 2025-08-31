# NetBox 单元测试 CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
set(TEST_PROJECT_NAME NetBoxTests)

# 查找线程库
find_package(Threads REQUIRED)

# 尝试查找系统安装的Google Test
find_package(GTest QUIET)

# 如果找不到GTest，使用FetchContent下载
if(NOT GTest_FOUND)
    message(STATUS "Google Test not found, downloading...")
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9fd7188ec8e5ef0a4c8bb8c.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    # 设置目标别名以保持一致性
    add_library(GTest::gtest ALIAS gtest)
    add_library(GTest::gtest_main ALIAS gtest_main)
else()
    message(STATUS "Using system Google Test")
endif()

# 包含头文件目录
include_directories(
    ${CMAKE_SOURCE_DIR}/NetFramework/include
    ${CMAKE_SOURCE_DIR}/Protocol/include
    ${CMAKE_SOURCE_DIR}/app/include
    ${CMAKE_SOURCE_DIR}/tests/include
)

# 创建测试库，包含所有需要测试的源文件
add_library(NetBoxTestLib STATIC
    # 基础组件
    ${CMAKE_SOURCE_DIR}/NetFramework/src/base/Logger.cpp
    ${CMAKE_SOURCE_DIR}/NetFramework/src/base/AsyncConsoleLogger.cpp
    ${CMAKE_SOURCE_DIR}/NetFramework/src/base/AsyncLogger.cpp
    ${CMAKE_SOURCE_DIR}/NetFramework/src/base/ThreadPool.cpp
    ${CMAKE_SOURCE_DIR}/NetFramework/src/base/DoubleLockThreadPool.cpp
    ${CMAKE_SOURCE_DIR}/NetFramework/src/base/SpdlogLogger.cpp
    
    # 工具类
    ${CMAKE_SOURCE_DIR}/NetFramework/src/util/ConfigReader.cpp
    ${CMAKE_SOURCE_DIR}/NetFramework/src/util/EnhancedConfigReader.cpp
    
    # IO多路复用
    ${CMAKE_SOURCE_DIR}/NetFramework/src/IO/EpollMultiplexer.cpp
    ${CMAKE_SOURCE_DIR}/NetFramework/src/IO/SelectMultiplexer.cpp
    ${CMAKE_SOURCE_DIR}/NetFramework/src/IO/PollMultiplexer.cpp
    
    # 服务器组件
    ${CMAKE_SOURCE_DIR}/NetFramework/src/server/TcpServer.cpp
    
    # 协议层
    ${CMAKE_SOURCE_DIR}/Protocol/src/ProtocolBase.cpp
    ${CMAKE_SOURCE_DIR}/Protocol/src/ProtocolRouter.cpp
    ${CMAKE_SOURCE_DIR}/Protocol/src/SimpleHeaderProtocol.cpp
    ${CMAKE_SOURCE_DIR}/Protocol/src/HttpProtocol.cpp
    ${CMAKE_SOURCE_DIR}/Protocol/src/RedisProtocol.cpp
    ${CMAKE_SOURCE_DIR}/Protocol/src/PureRedisProtocol.cpp
    ${CMAKE_SOURCE_DIR}/Protocol/src/ProtocolFactory.cpp
    
    # 应用层
    ${CMAKE_SOURCE_DIR}/app/src/ApplicationServer.cpp
    ${CMAKE_SOURCE_DIR}/app/src/server.cpp
    ${CMAKE_SOURCE_DIR}/app/src/RedisApplicationServer.cpp
    ${CMAKE_SOURCE_DIR}/app/src/DirectRedisServer.cpp
    
    # 插件化框架
    ${CMAKE_SOURCE_DIR}/NetFramework/src/app/ApplicationRegistry.cpp
)

# 链接依赖库
target_link_libraries(NetBoxTestLib PRIVATE 
    Threads::Threads 
    spdlog::spdlog 
    fmt::fmt
)

# 基础组件测试
add_executable(test_base
    unit/base/test_logger.cpp
    unit/base/test_async_logger.cpp
    unit/base/test_thread_pool.cpp
    unit/base/test_double_lock_thread_pool.cpp
)
target_link_libraries(test_base NetBoxTestLib GTest::gtest GTest::gtest_main)

# 工具类测试
add_executable(test_util
    unit/util/test_config_reader.cpp
    unit/util/test_enhanced_config_reader.cpp
)
target_link_libraries(test_util NetBoxTestLib GTest::gtest GTest::gtest_main)

# IO多路复用测试
add_executable(test_io
    unit/io/test_io_multiplexer.cpp
    unit/io/test_epoll_multiplexer.cpp
    unit/io/test_select_multiplexer.cpp
    unit/io/test_poll_multiplexer.cpp
)
target_link_libraries(test_io NetBoxTestLib GTest::gtest GTest::gtest_main)

# 协议层测试
add_executable(test_protocol
    unit/protocol/test_protocol_base.cpp
    unit/protocol/test_protocol_router.cpp
    unit/protocol/test_simple_header_protocol.cpp
    unit/protocol/test_http_protocol.cpp
    unit/protocol/test_redis_protocol.cpp
)
target_link_libraries(test_protocol NetBoxTestLib GTest::gtest GTest::gtest_main)

# 服务器组件测试
add_executable(test_server
    unit/server/test_tcp_server.cpp
)
target_link_libraries(test_server NetBoxTestLib GTest::gtest GTest::gtest_main)

# 应用层测试
add_executable(test_app
    unit/app/test_application_server.cpp
    unit/app/test_echo_server.cpp
    unit/app/test_redis_application_server.cpp
    unit/app/test_application_registry.cpp
)
target_link_libraries(test_app NetBoxTestLib GTest::gtest GTest::gtest_main)

# 集成测试
add_executable(test_integration
    integration/test_echo_integration.cpp
    integration/test_redis_integration.cpp
    integration/test_protocol_integration.cpp
)
target_link_libraries(test_integration NetBoxTestLib GTest::gtest GTest::gtest_main)

# 性能测试
add_executable(test_performance
    performance/test_thread_pool_performance.cpp
    performance/test_io_performance.cpp
    performance/test_protocol_performance.cpp
    performance/test_framework_benchmark.cpp
)
target_link_libraries(test_performance NetBoxTestLib GTest::gtest GTest::gtest_main)

# 添加测试目标
enable_testing()

add_test(NAME BaseTests COMMAND test_base)
add_test(NAME UtilTests COMMAND test_util)
add_test(NAME IOTests COMMAND test_io)
add_test(NAME ProtocolTests COMMAND test_protocol)
add_test(NAME ServerTests COMMAND test_server)
add_test(NAME AppTests COMMAND test_app)
add_test(NAME IntegrationTests COMMAND test_integration)
add_test(NAME PerformanceTests COMMAND test_performance)

# 设置测试输出目录
set_target_properties(
    test_base test_util test_io test_protocol test_server test_app test_integration test_performance
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests/bin
)

# 创建运行所有测试的目标
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS test_base test_util test_io test_protocol test_server test_app test_integration test_performance
    COMMENT "Running all unit tests"
)

# 创建生成测试报告的目标
add_custom_target(test_report
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --output-junit ${CMAKE_BINARY_DIR}/test_results.xml
    DEPENDS test_base test_util test_io test_protocol test_server test_app test_integration test_performance
    COMMENT "Generating test report"
)
