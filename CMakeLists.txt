cmake_minimum_required(VERSION 3.23)

project(unvm VERSION 0.0.1)

string(TOUPPER "${CMAKE_SYSTEM_NAME}" PLATFORM_SYSTEM_NAME)
string(TOUPPER "${CMAKE_SYSTEM_PROCESSOR}" PLATFORM_SYSTEM_PROCESSOR)

message(STATUS "System: ${PLATFORM_SYSTEM_NAME}")
message(STATUS "Processor: ${PLATFORM_SYSTEM_PROCESSOR}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ZLIB_BUILD_TESTING OFF)

add_subdirectory(third_party/zlib)

set(ENABLE_ZLIB ON)
set(ENABLE_TEST OFF)
set(ENABLE_COVERAGE OFF)

set(ENABLE_TAR OFF)
set(ENABLE_CPIO OFF)
set(ENABLE_CAT OFF)
set(ENABLE_UNZIP OFF)

add_subdirectory(third_party/libarchive)

file(GLOB_RECURSE SRC "src/*.cxx")
add_executable(unvm ${SRC})
target_include_directories(unvm PRIVATE "include")
target_link_libraries(unvm PRIVATE archive)
target_compile_definitions(unvm PRIVATE "SYSTEM_${PLATFORM_SYSTEM_NAME}" "ARCH_${PLATFORM_SYSTEM_PROCESSOR}")

if (WIN32)
    target_link_libraries(unvm PRIVATE ws2_32 bcrypt xmllite uuid kernel32 user32 gdi32 winspool shell32 ole32 oleaut32 comdlg32 advapi32)
endif()

if (MINGW)
    target_link_options(unvm PRIVATE 
        -static-libgcc
        -static-libstdc++
    )
    target_link_libraries(unvm PRIVATE pthread)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_link_options(unvm PRIVATE -fuse-ld=lld)
endif()

install(TARGETS unvm)
