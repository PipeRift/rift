
include(FetchContent)

# Submodule libraries
option(CLI11_BUILD_TESTS "Build CLI11 tests" OFF)
add_subdirectory(CLI11)
add_subdirectory(EABase)
add_subdirectory(fmt)
add_subdirectory(glm)
add_subdirectory(SparseMap)
add_subdirectory(RobinMap)
add_subdirectory(Taskflow)
option(SPDLOG_FMT_EXTERNAL "Use external fmt library instead of bundled" ON)
add_subdirectory(spdlog)
option(JSON_BuildTests "Build the unit tests when BUILD_TESTING is enabled." OFF)
add_subdirectory(nlohmann_json)

# TRACY
add_library(Tracy STATIC)
target_include_directories(Tracy PUBLIC Tracy)
target_sources(Tracy PRIVATE Tracy/TracyClient.cpp)
target_compile_features(Tracy PUBLIC cxx_std_17)
if(LINUX)
    find_library(DLPATH dl)
    target_link_libraries(Tracy PRIVATE ${DLPATH})
endif()

# EASTL
file(GLOB_RECURSE EASTL_SOURCES CONFIGURE_DEPENDS EASTL/source/*.cpp)
add_library(EASTL STATIC ${EASTL_SOURCES})
target_include_directories(EASTL PUBLIC EASTL/include)
target_link_libraries(EASTL EABase)


if(WINDOWS)
    target_compile_definitions(Tracy PRIVATE _WIN32=1)
elseif(LINUX)
    target_compile_definitions(Tracy PRIVATE __linux__=1)
endif()

if(VCLANG_ENABLE_PROFILER)
    target_compile_definitions(Tracy PUBLIC TRACY_ENABLE=1)
endif()
