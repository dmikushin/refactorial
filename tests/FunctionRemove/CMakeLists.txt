cmake_minimum_required(VERSION 3.1)

project(function_remove CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

configure_file(foo.cpp.in foo.cpp)
configure_file(foo.h.in foo.h)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_BINARY_DIR}/foo.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(test
     DEPENDS ${PROJECT_NAME}
     COMMAND XUNUSED_VERBOSE=1 ${REFACTORIAL} --spec ${CMAKE_CURRENT_SOURCE_DIR}/function_remove.yml --print --apply ./compile_commands.json
     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
     COMMENT "Testing FunctionRemove")

