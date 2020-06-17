
add_library(Tracy STATIC)
target_include_directories(Tracy PUBLIC Include Include/tracy/common Include/tracy/client)

file(GLOB_RECURSE TRACY_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp)
target_sources(Tracy PRIVATE ${TRACY_SOURCE_FILES})

target_compile_features(Tracy PUBLIC cxx_std_17)

if(ENABLE_PROFILER)
    target_compile_definitions(Tracy PUBLIC TRACY_ENABLE=1)
endif()
