
file(GLOB_RECURSE CORE_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)

add_library(Core STATIC ${CORE_SOURCE_FILES})
target_include_directories(Core PUBLIC Include)
target_compile_features(Core PUBLIC cxx_std_17)
target_define_platform(Core)

conan_cmake_run(REQUIRES eastl/3.16.05 BASIC_SETUP CMAKE_TARGETS BUILD missing)

target_link_libraries(Core PUBLIC CONAN_PKG::eastl Tracy GLM)

install(TARGETS Core
    EXPORT VCLangTargets
    LIBRARY DESTINATION Lib
    ARCHIVE DESTINATION Lib
    RUNTIME DESTINATION Bin
    INCLUDES DESTINATION Include
)