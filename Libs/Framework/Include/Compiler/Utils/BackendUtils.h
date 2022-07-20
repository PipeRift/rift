// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Compiler/Backend.h"
#include "Compiler/Compiler.h"

#include <Pipe/Memory/OwnPtr.h>
#include <Pipe/Reflect/ClassType.h>


namespace rift::compiler
{
	TArray<ClassType*> GetBackendTypes();
	TArray<TOwnPtr<Backend>> CreateBackends();
}    // namespace rift::compiler
