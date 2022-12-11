
function(rift_module target)
    pipe_target_define_platform(${target})
    pipe_target_shared_output_directory(${target})
    pipe_target_enable_CPP20(${target})
endfunction(rift_module)

function(rift_enable_module_resources target)
    add_custom_command(TARGET ${target} POST_BUILD COMMAND
        ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Resources" "$<TARGET_FILE_DIR:${target}>/Resources"
    )
endfunction(rift_enable_module_resources)
