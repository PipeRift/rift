
include(FetchContent)

# Sparse-map & Robin-map
add_subdirectory(SparseMap)
add_subdirectory(RobinMap)

add_subdirectory(Taskflow)

add_subdirectory(CLI11)

# TRACY
add_library(Tracy STATIC)
target_include_directories(Tracy PUBLIC Tracy)
target_sources(Tracy PRIVATE Tracy/TracyClient.cpp)
target_compile_features(Tracy PUBLIC cxx_std_17)

if(WINDOWS)
    target_compile_definitions(Tracy PRIVATE _WIN32=1)
elseif(LINUX)
    target_compile_definitions(Tracy PRIVATE __linux__=1)
endif()

if(ENABLE_PROFILER)
    target_compile_definitions(Tracy PUBLIC TRACY_ENABLE=1)
endif()
