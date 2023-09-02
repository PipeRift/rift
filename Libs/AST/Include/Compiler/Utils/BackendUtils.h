// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "Compiler/Backend.h"
#include "Compiler/Compiler.h"

#include <Pipe/Memory/OwnPtr.h>
#include <Pipe/Reflect/ClassType.h>


namespace rift
{
	TArray<ClassType*> GetBackendTypes();
	TArray<TOwnPtr<Backend>> CreateBackends();
}    // namespace rift
