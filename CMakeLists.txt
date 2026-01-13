cmake_minimum_required(VERSION 3.23)

project(unvm)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory("third_party/bit7z")

file(GLOB_RECURSE SRC "src/*.cxx")
add_executable(unvm ${SRC})
target_include_directories(unvm PRIVATE "include")
target_link_libraries(unvm PRIVATE winhttp bit7z)
