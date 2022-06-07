// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Compiler/Backend.h"
#include "Compiler/Context.h"

#include <Pipe/Memory/OwnPtr.h>
#include <Pipe/Reflect/ClassType.h>


namespace rift::Compiler
{
	TArray<ClassType*> GetBackendTypes();
	TArray<TOwnPtr<Backend>> CreateBackends();

	TOwnPtr<Context> CreateBackend(ClassType* backendClass);
}    // namespace rift::Compiler
