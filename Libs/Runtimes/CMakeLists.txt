# Copyright 2015-2023 Piperift - All rights reserved

rift_runtime_module(Files)
target_link_libraries(RiftRuntimeFiles PUBLIC Pipe)

rift_runtime_module(IO)
target_link_libraries(RiftRuntimeIO PUBLIC Pipe)

rift_runtime_module(Math)
target_link_libraries(RiftRuntimeMath PUBLIC Pipe)
