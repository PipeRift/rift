
add_library(RiftBackendLLVM STATIC)
add_library(rift::Backend::LLVM ALIAS RiftBackendLLVM)

target_include_directories(RiftBackendLLVM PUBLIC Include)
file(GLOB_RECURSE BACKEND_LLVM_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(RiftBackendLLVM PRIVATE ${BACKEND_LLVM_SOURCE_FILES})

target_link_libraries(RiftBackendLLVM PUBLIC rift::Rift)
target_link_libraries(RiftBackendLLVM PRIVATE
    RiftLLVM
    rift::Bindings::Native
)
target_compile_definitions(RiftBackendLLVM PRIVATE RIFT_LLVM_LINKER="${RIFT_LLVM_LINKER}" RIFT_LLVM_LINKER_PATH="${RIFT_LLVM_LINKER_PATH}")

rift_module(RiftBackendLLVM)
set_target_properties(RiftBackendLLVM PROPERTIES FOLDER Rift)

install(TARGETS RiftBackendLLVM
    EXPORT RiftTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)
