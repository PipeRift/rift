
file(GLOB_RECURSE CORE_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)

add_library(Core STATIC)
target_include_directories(Core PUBLIC Include)
target_sources(Core PRIVATE ${CORE_SOURCE_FILES})
set_property(TARGET Core PROPERTY CXX_STANDARD 17)
target_define_platform(Core)

target_link_libraries(Core PUBLIC
    EASTL
    date::date
    nlohmann_json
    glm::glm_static
    Tracy
    tsl::robin_map
    tsl::sparse_map
    Taskflow
    fmt
    robin_hood
    rpmalloc
)
target_link_libraries(Core PRIVATE
    spdlog
)

target_shared_output_directory(Core)

install(TARGETS Core
    EXPORT VCLangTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)