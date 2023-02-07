# Copyright 2015-2023 Piperift - All rights reserved

add_library(RiftBackendLLVM STATIC)
rift_compiler_module(RiftBackendLLVM)

target_link_libraries(RiftBackendLLVM PUBLIC Rift)
target_link_libraries(RiftBackendLLVM PRIVATE
    RiftLLVM
    RiftBindingNative
)
target_compile_definitions(RiftBackendLLVM PRIVATE RIFT_LLVM_LINKER="${RIFT_LLVM_LINKER}" RIFT_LLVM_LINKER_PATH="${RIFT_LLVM_LINKER_PATH}")


# Copy LLVM linker
if (PLATFORM_WINDOWS)
    set(RIFT_LLVM_LINKER lld-link.exe)
elseif(PLATFORM_LINUX)
    set(RIFT_LLVM_LINKER ld.lld)
elseif(PLATFORM_MACOS)
    set(RIFT_LLVM_LINKER ld64.lld)
endif()

file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/Bin/LLVM)
file(COPY ${RIFT_LLVM_BIN_PATH}/bin/${RIFT_LLVM_LINKER} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Bin/LLVM)
set(RIFT_LLVM_LINKER_PATH LLVM/${RIFT_LLVM_LINKER})

add_custom_command(TARGET RiftBackendLLVM POST_BUILD COMMAND
    ${CMAKE_COMMAND} -E copy "${RIFT_LLVM_BIN_PATH}/bin/${RIFT_LLVM_LINKER}" "${CMAKE_CURRENT_BINARY_DIR}/Bin/LLVM/${RIFT_LLVM_LINKER}"
)
