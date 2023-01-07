# Copyright 2015-2023 Piperift - All rights reserved

set(PIPE_BUILD_TESTS ${RIFT_BUILD_TESTS} CACHE BOOL "Build Pipe tests")
add_subdirectory(Pipe)

add_subdirectory(Framework)
add_subdirectory(Backends/LLVM)
add_subdirectory(Bindings/Native)

add_subdirectory(UI)
add_subdirectory(Views/Graph)
add_subdirectory(Editor)

add_subdirectory(Runtimes)
