# 基础示例
add_executable(simple_server basic/simple_server.cpp)
target_link_libraries(simple_server TestProject_framework)

# 自定义协议示例
add_executable(custom_protocol custom_protocol/custom_protocol.cpp)
target_link_libraries(custom_protocol TestProject_framework)

# 高级示例
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/advanced")
    file(GLOB ADVANCED_EXAMPLES "advanced/*.cpp")
    foreach(EXAMPLE ${ADVANCED_EXAMPLES})
        get_filename_component(EXAMPLE_NAME ${EXAMPLE} NAME_WE)
        add_executable(${EXAMPLE_NAME} ${EXAMPLE})
        target_link_libraries(${EXAMPLE_NAME} TestProject_framework)
    endforeach()
endif()
