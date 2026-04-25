set(BINARY_DIR "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin")
file(MAKE_DIRECTORY "${BINARY_DIR}")

function (create_link name)
    if (WIN32)
        message(STATUS "creating hardlink for ${name}")
        execute_process(COMMAND
                ${CMAKE_COMMAND} -E chdir ${BINARY_DIR}
                ${CMAKE_COMMAND} -E create_hardlink unvm.exe ${name}.exe
            RESULT_VARIABLE result
        )

        if (NOT result EQUAL 0)
            message(WARNING "failed to create hardlink for ${name}, falling back to copy")
            execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${BINARY_DIR}/unvm.exe ${BINARY_DIR}/${name}.exe)
        endif ()
    elseif (UNIX)
        message(STATUS "creating symlink for ${name}")
        execute_process(COMMAND
                ${CMAKE_COMMAND} -E chdir ${BINARY_DIR}
                ${CMAKE_COMMAND} -E create_symlink unvm ${name}
            RESULT_VARIABLE result
        )

        if (NOT result EQUAL 0)
            message(WARNING "failed to create symlink for ${name}")
        endif ()
    endif ()
endfunction ()

create_link(node)
create_link(npm)
create_link(npx)
