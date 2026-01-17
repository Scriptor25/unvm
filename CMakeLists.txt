cmake_minimum_required(VERSION 3.23)

project(unvm)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRC "src/*.cxx")
add_executable(unvm ${SRC})
target_include_directories(unvm PRIVATE "include")

if (WIN32)
    target_link_libraries(unvm PRIVATE ws2_32)
endif()

if (MINGW)
    target_link_options(unvm PRIVATE 
        -static-libgcc
        -static-libstdc++
    )
endif()

install(TARGETS unvm)
