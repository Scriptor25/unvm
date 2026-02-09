cmake_minimum_required(VERSION 3.31)

project(unvm
        LANGUAGES C CXX
        VERSION "${UNVM_PROJECT_VERSION}")

### Find dependencies ###

find_package(OpenSSL REQUIRED)
find_package(LibArchive REQUIRED)
find_package(ZLIB REQUIRED)

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

set(CERT_INPUT ${CMAKE_SOURCE_DIR}/ssl/cacert.pem)
set(CERT_OUTPUT ${CMAKE_BINARY_DIR}/ssl/cert.h)

add_custom_command(
        OUTPUT ${CERT_OUTPUT}
        COMMAND xxd -i -n cert_data ${CERT_INPUT} ${CERT_OUTPUT}
        DEPENDS ${CERT_INPUT}
        COMMENT "embedding cert.pem with xxd"
)
add_custom_target(generate_cert DEPENDS ${CERT_OUTPUT})

file(GLOB_RECURSE SRC "src/*.cxx")

add_executable(unvm ${SRC})

add_dependencies(unvm generate_cert)

target_include_directories(unvm PRIVATE ${CMAKE_BINARY_DIR} "include")

target_link_libraries(unvm PRIVATE
        OpenSSL::SSL
        OpenSSL::Crypto
        LibArchive::LibArchive
        ZLIB::ZLIB
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

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)

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

if (APPLE)
    install(TARGETS unvm
            RUNTIME_DEPENDENCIES
            FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR})
elseif (WIN32)
    install(TARGETS unvm
            RUNTIME_DEPENDENCIES
            PRE_EXCLUDE_REGEXES
            "api-ms-win-.*"
            "ext-ms-.*"
            "kernel32\\.dll"
            "user32\\.dll"
            "gdi32\\.dll"
            "advapi32\\.dll"
            "shell32\\.dll"
            "ole32\\.dll"
            "oleaut32\\.dll"
            "ws2_32\\.dll"
            "ntdll\\.dll"
            POST_EXCLUDE_REGEXES
            ".*system32/.*"
            ".*System32/.*"
            RUNTIME DESTINATION bin)
else ()
    install(TARGETS unvm RUNTIME_DEPENDENCIES)
endif ()

if (ENABLE_COMPLETIONS)
    install(FILES completions/unvm-completion.sh
            DESTINATION share/bash-completion/completions
            RENAME unvm
    )
endif ()

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
elseif (UNIX)
    set(CPACK_RPM_PACKAGE_REQUIRES "openssl, libarchive, zlib")

    string(APPEND CPACK_GENERATOR ";STGZ;RPM")
endif ()

include(CPack)
