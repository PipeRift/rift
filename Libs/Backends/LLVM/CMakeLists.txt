# Copyright 2015-2023 Piperift - All rights reserved

add_library(RiftBackendLLVM STATIC)
rift_compiler_module(RiftBackendLLVM)

target_link_libraries(RiftBackendLLVM PUBLIC RiftAST)
target_link_libraries(RiftBackendLLVM PRIVATE
    RiftLLVM
    RiftBindingNative
)


# LLVM linker
if (PLATFORM_WINDOWS)
    set(RIFT_LLVM_LINKER lld-link.exe)
elseif(PLATFORM_LINUX)
    set(RIFT_LLVM_LINKER ld.lld)
elseif(PLATFORM_MACOS)
    set(RIFT_LLVM_LINKER ld64.lld)
endif()

set(RIFT_LLVM_LINKER_PATH LLVM/${RIFT_LLVM_LINKER})

target_compile_definitions(RiftBackendLLVM PRIVATE RIFT_LLVM_LINKER="${RIFT_LLVM_LINKER}" RIFT_LLVM_LINKER_PATH="${RIFT_LLVM_LINKER_PATH}")

add_custom_command(TARGET RiftBackendLLVM POST_BUILD COMMAND
    ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/Bin/LLVM"
)
add_custom_command(TARGET RiftBackendLLVM POST_BUILD COMMAND
    ${CMAKE_COMMAND} -E copy "${RIFT_LLVM_BIN_PATH}/bin/${RIFT_LLVM_LINKER}" "${CMAKE_BINARY_DIR}/Bin/LLVM/${RIFT_LLVM_LINKER}"
)

