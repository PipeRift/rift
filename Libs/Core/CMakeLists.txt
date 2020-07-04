
file(GLOB_RECURSE CORE_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)

add_library(Core STATIC ${CORE_SOURCE_FILES})
target_include_directories(Core PUBLIC Include)
target_compile_features(Core PUBLIC cxx_std_17)
target_define_platform(Core)

target_link_libraries(Core PUBLIC
    CONAN_PKG::eastl
    CONAN_PKG::glm
    CONAN_PKG::spdlog
    CONAN_PKG::nlohmann_json
    CONAN_PKG::date
    CONAN_PKG::fmt
    Tracy
    tsl::robin_map
    tsl::sparse_map
    Taskflow
)

target_shared_output_directory(Core)

install(TARGETS Core
    EXPORT VCLangTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)