cmake_minimum_required(VERSION 3.20)

project(execute_binary_code)

add_library(execute_binary_code
    execute_binary_code.hpp
    execute_binary_code.cpp
)
target_include_directories(execute_binary_code PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(execute_binary_code PUBLIC cxx_std_23)

add_executable(execute_binary_code_test
    main.cpp)

target_link_libraries(execute_binary_code_test PUBLIC execute_binary_code)
