# Copyright 2015-2023 Piperift - All rights reserved

add_subdirectory(AST)
add_subdirectory(Backends/MIR)
add_subdirectory(Bindings/Native)

# Views/Graph provides a compiler module (GraphViewModule) used by the CLI,
# so it is always built. Its editor part is gated below.
add_subdirectory(Views/Graph)

if(RIFT_BUILD_EDITOR)
    add_subdirectory(UI)
    add_subdirectory(Editor)
endif()

add_subdirectory(Runtimes)
