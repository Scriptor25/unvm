cmake_minimum_required(VERSION 3.28)

project(unvm
        LANGUAGES C CXX
        VERSION "${UNVM_PROJECT_VERSION}"
)

set(PROJECT_NAME "UNVM")
set(PROJECT_TITLE "Universal Node Version Manager")

string(TIMESTAMP BUILD_DATE "%d/%m/%Y")
set(PROJECT_BUILD_DATE "${BUILD_DATE}")

### Find dependencies ###

find_package(OpenSSL REQUIRED COMPONENTS Crypto SSL)
find_package(LibArchive REQUIRED)
find_package(ZLIB REQUIRED)
find_package(LibLZMA REQUIRED)

### Options ###

option(ENABLE_COMPLETIONS "install shell completion scripts" ON)

### Platform info ###

string(TOUPPER "${CMAKE_SYSTEM_NAME}" PLATFORM_SYSTEM_NAME)
string(TOUPPER "${CMAKE_SYSTEM_PROCESSOR}" PLATFORM_SYSTEM_PROCESSOR)

message(STATUS "System: ${PLATFORM_SYSTEM_NAME}")
message(STATUS "Processor: ${PLATFORM_SYSTEM_PROCESSOR}")

### Subdirectories ###

add_subdirectory(deps/toolkit)

### Targets ###

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

function(add_xxd NAME SRC DST)

    add_custom_command(
            OUTPUT "${DST}"
            COMMAND xxd -i -n "${NAME}" "${SRC}" "${DST}"
            DEPENDS "${SRC}"
            COMMENT "embedding ${NAME} with xxd"
    )
    add_custom_target(${NAME} DEPENDS "${DST}")

endfunction()

add_xxd(cacert "${CMAKE_SOURCE_DIR}/data/cacert.pem" "${CMAKE_CURRENT_BINARY_DIR}/data/cacert.h")
add_xxd(keyring "${CMAKE_SOURCE_DIR}/data/keyring.pgp" "${CMAKE_CURRENT_BINARY_DIR}/data/keyring.h")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/definitions/version.h.in" "${CMAKE_CURRENT_BINARY_DIR}/version.h" @ONLY)

file(GLOB_RECURSE SRC "src/*.cxx")
add_executable(unvm ${SRC})

add_dependencies(unvm cacert keyring)

target_include_directories(unvm PRIVATE "${CMAKE_CURRENT_BINARY_DIR}" "include")

# target_compile_options(unvm PRIVATE "-fsanitize=address")
# target_link_options(unvm PRIVATE "-fsanitize=address" "-static-libasan")

target_link_libraries(unvm PRIVATE
        toolkit::core
        toolkit::json

        OpenSSL::Crypto
        OpenSSL::SSL

        LibArchive::LibArchive

        ZLIB::ZLIB

        LibLZMA::LibLZMA
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

set(COMMON_LICENSE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
        "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt"
        "${CMAKE_CURRENT_SOURCE_DIR}/THIRD_PARTY_NOTICES.txt"
)

if (WIN32)
    install(TARGETS unvm
            RUNTIME_DEPENDENCIES
            PRE_EXCLUDE_REGEXES
            "api-ms-.*"
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
            RUNTIME DESTINATION bin
    )

    install(FILES ${COMMON_LICENSE_FILES} DESTINATION "licenses")
elseif (APPLE)
    install(TARGETS unvm
            RUNTIME_DEPENDENCIES
            FRAMEWORK DESTINATION lib
    )

    install(FILES ${COMMON_LICENSE_FILES} DESTINATION "unvm.app/Contents/Resources")
elseif (UNIX)
    install(TARGETS unvm)

    install(FILES ${COMMON_LICENSE_FILES} DESTINATION "share/doc/unvm/licenses")
endif ()

install(SCRIPT "cmake/create_links.cmake")

if (ENABLE_COMPLETIONS)
    install(FILES completions/unvm-completion.sh
            DESTINATION share/bash-completion/completions
            RENAME unvm
    )
endif ()

### CPack Config ###

set(CPACK_PACKAGE_VENDOR "Scriptor25")
set(CPACK_PACKAGE_DESCRIPTION_FILE)
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
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
