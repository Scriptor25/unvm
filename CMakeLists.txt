cmake_minimum_required(VERSION 3.23)

project(unvm VERSION 0.0.3)

### Find dependencies ###

find_package(OpenSSL REQUIRED)
find_package(LibArchive REQUIRED)

### Options ###

option(ENABLE_COMPLETIONS "install shell completion scripts" ON)

### Platform info ###

string(TOUPPER "${CMAKE_SYSTEM_NAME}" PLATFORM_SYSTEM_NAME)
string(TOUPPER "${CMAKE_SYSTEM_PROCESSOR}" PLATFORM_SYSTEM_PROCESSOR)

message(STATUS "System: ${PLATFORM_SYSTEM_NAME}")
message(STATUS "Processor: ${PLATFORM_SYSTEM_PROCESSOR}")

### Targets ###

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRC "src/*.cxx")

add_executable(unvm ${SRC})

target_include_directories(unvm PRIVATE "include")

target_link_libraries(unvm PRIVATE
        OpenSSL::SSL
        OpenSSL::Crypto
        LibArchive::LibArchive
)

target_compile_definitions(unvm PRIVATE
        "SYSTEM_${PLATFORM_SYSTEM_NAME}"
        "ARCH_${PLATFORM_SYSTEM_PROCESSOR}"
)

### Platform Config ###

if (APPLE)
    set(CMAKE_INSTALL_RPATH "@loader_path/../lib")
    set(CMAKE_BUILD_RPATH "@loader_path/../lib")
elseif (UNIX)
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
    set(CMAKE_BUILD_RPATH "$ORIGIN/../lib")
endif ()

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if (WIN32)
    target_link_libraries(unvm PRIVATE
            ws2_32
            bcrypt
            xmllite
            uuid
            kernel32
            user32
            gdi32
            winspool
            shell32
            ole32
            oleaut32
            comdlg32
            advapi32
            crypt32
    )
endif ()

if (MINGW)
    target_link_options(unvm PRIVATE
            -static-libgcc
            -static-libstdc++
    )
endif ()

### Install Config ###

install(TARGETS unvm
        RUNTIME_DEPENDENCY_SET dependencies
        RUNTIME DESTINATION bin
)

if (WIN32)
    set(RUNTIME_DEPENDENCY_DESTINATION bin)
else ()
    set(RUNTIME_DEPENDENCY_DESTINATION lib)
endif ()

install(RUNTIME_DEPENDENCY_SET dependencies
        DESTINATION ${RUNTIME_DEPENDENCY_DESTINATION}
        PRE_EXCLUDE_REGEXES

        #### Windows ####
        "api-ms-"
        "ext-ms-"
        "^C:/Windows"

        #### Linux ####
        "^/lib"
        "^/usr/lib"
        "^/lib64"
        "^/usr/lib64"

        #### MacOS ####
        "^/System/Library"
        "^/usr/lib"

        POST_EXCLUDE_REGEXES

        #### Windows ####
        "kernel32.dll"
        "user32.dll"
        "gdi32.dll"
        "advapi32.dll"
        "msvcp.*.dll"
        "vcruntime.*.dll"
        "ucrtbase.dll"

        #### Linux ####

        "ld-linux"

        #### Unix ####

        "libc\\.so"
        "libm\\.so"
        "librt\\.so"
        "libpthread\\.so"
        "libdl\\.so"

        #### GNU ####

        "libgcc_s.*"
        "libstdc\\+\\+.*"
)

if (ENABLE_COMPLETIONS)
    install(FILES completions/unvm-completion.sh
            DESTINATION share/bash-completion/completions
            RENAME unvm
    )
endif ()

install(FILES ssl/nodejs.pem
        DESTINATION ${CMAKE_INSTALL_BINDIR}/ssl
)

### CPack Config ###

set(CPACK_PACKAGE_VENDOR "Scriptor25")
set(CPACK_PACKAGE_DESCRIPTION_FILE)
set(CPACK_RESOURCE_FILE_LICENSE)
set(CPACK_RESOURCE_FILE_README)
set(CPACK_GENERATOR "TGZ")
set(CPACK_SOURCE_GENERATOR "TGZ")

if (WIN32)
    string(APPEND CPACK_GENERATOR ";ZIP;INNOSETUP")
    string(APPEND CPACK_SOURCE_GENERATOR ";ZIP")
elseif (APPLE)
    string(APPEND CPACK_GENERATOR ";STGZ")
else ()
    string(APPEND CPACK_GENERATOR ";STGZ;RPM")
endif ()

include(CPack)
