
function(rift_module target)
    pipe_target_define_platform(${target})
    pipe_target_shared_output_directory(${target})
    pipe_target_enable_CPP20(${target})
    set_target_properties(${target} PROPERTIES FOLDER Rift)
endfunction(rift_module)

function(rift_compiler_module target)
    rift_module(${target})

    target_include_directories(${target} PUBLIC Compiler/Include)
    file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS Compiler/Src/*.cpp Compiler/Src/*.h)
    target_sources(${target} PRIVATE ${SOURCE_FILES})
    set(RIFT_COMPILER_MODULES ${RIFT_COMPILER_MODULES} ${target} PARENT_SCOPE)
    target_link_libraries(RiftCompilerModules INTERFACE ${target})
endfunction(rift_compiler_module)

function(rift_editor_module target)
    rift_module(${target})

    target_include_directories(${target} PUBLIC Editor/Include)
    file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS Editor/Src/*.cpp Editor/Src/*.h)
    target_sources(${target} PRIVATE ${SOURCE_FILES})
    target_link_libraries(RiftEditorModules INTERFACE ${target})
endfunction(rift_editor_module)

function(rift_enable_module_resources target)
    add_custom_command(TARGET ${target} POST_BUILD COMMAND
        ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Resources" "$<TARGET_FILE_DIR:${target}>/Resources"
    )
endfunction(rift_enable_module_resources)
