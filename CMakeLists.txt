cmake_minimum_required(VERSION 3.23)

project(unvm VERSION 0.0.1)

find_package(OpenSSL REQUIRED)
find_package(LibArchive REQUIRED)

option(ENABLE_COMPLETIONS "install shell completion scripts" ON)

string(TOUPPER "${CMAKE_SYSTEM_NAME}" PLATFORM_SYSTEM_NAME)
string(TOUPPER "${CMAKE_SYSTEM_PROCESSOR}" PLATFORM_SYSTEM_PROCESSOR)

message(STATUS "System: ${PLATFORM_SYSTEM_NAME}")
message(STATUS "Processor: ${PLATFORM_SYSTEM_PROCESSOR}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRC "src/*.cxx")
add_executable(unvm ${SRC})
target_include_directories(unvm PRIVATE "include")
target_link_libraries(unvm PRIVATE ssl crypto archive)
target_compile_definitions(unvm PRIVATE "SYSTEM_${PLATFORM_SYSTEM_NAME}" "ARCH_${PLATFORM_SYSTEM_PROCESSOR}")

if (WIN32)
    target_link_libraries(unvm PRIVATE ws2_32 bcrypt xmllite uuid kernel32 user32 gdi32 winspool shell32 ole32 oleaut32 comdlg32 advapi32)
endif ()

if (MINGW)
    target_link_options(unvm PRIVATE -static-libgcc -static-libstdc++)
    target_link_libraries(unvm PRIVATE pthread)
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_link_options(unvm PRIVATE -fuse-ld=lld)
endif ()

install(TARGETS unvm)

if (ENABLE_COMPLETIONS)
    install(FILES completions/unvm-completion.sh
            DESTINATION share/bash-completion/completions
            RENAME unvm)
endif ()

set(CPACK_PACKAGE_VENDOR "Scriptor25")
set(CPACK_PACKAGE_DESCRIPTION_FILE)
set(CPACK_RESOURCE_FILE_LICENSE)
set(CPACK_RESOURCE_FILE_README)
set(CPACK_GENERATOR "TGZ")
set(CPACK_SOURCE_GENERATOR "TGZ")

if (WIN32)
    string(APPEND CPACK_GENERATOR ";ZIP;INNOSETUP")
    string(APPEND CPACK_SOURCE_GENERATOR ";ZIP")
else ()
    string(APPEND CPACK_GENERATOR ";STGZ;RPM")
endif ()

include(CPack)
