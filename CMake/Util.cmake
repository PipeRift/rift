if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(WINDOWS TRUE)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(LINUX TRUE)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX TRUE)
endif()

function(target_define_platform target)
    if (WINDOWS)
        target_compile_definitions(${target} PRIVATE PLATFORM_WINDOWS=1)
    elseif (LINUX)
        target_compile_definitions(${target} PRIVATE PLATFORM_LINUX=1)
    elseif (MACOSX)
        target_compile_definitions(${target} PRIVATE PLATFORM_MAC=1)
    endif()

    target_compile_definitions(${target} PRIVATE
        $<$<CONFIG:Debug>:BUILD_DEBUG>
        $<$<CONFIG:Release>:BUILD_RELEASE>
    )
endfunction()

function(target_shared_output_directory target)
    set_target_properties(${target}
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
endfunction()
