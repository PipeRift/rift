// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Compiler/Backend.h"
#include "Compiler/Context.h"

#include <Memory/OwnPtr.h>
#include <Reflection/ClassType.h>


namespace rift::Compiler
{
	TArray<refl::ClassType*> GetBackendTypes();
	TArray<TOwnPtr<Backend>> CreateBackends();

	TOwnPtr<Context> CreateBackend(refl::ClassType* backendClass);
}    // namespace rift::Compiler
