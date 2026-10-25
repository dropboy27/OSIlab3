cmake_minimum_required(VERSION 3.10)
project(OSIlab3 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(main main.c carlo.c)

target_compile_options(main PRIVATE -Wall -Wextra)

find_package(Threads REQUIRED)
target_link_libraries(main PRIVATE Threads::Threads)