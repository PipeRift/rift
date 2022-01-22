// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Compiler/Backend.h"
#include "Compiler/Context.h"

#include <Memory/OwnPtr.h>
#include <Reflection/Static/ClassType.h>


namespace Rift::Compiler
{
	TArray<Refl::ClassType*> GetBackendTypes();
	TArray<TOwnPtr<Backend>> CreateBackends();

	TOwnPtr<Context> CreateBackend(Refl::ClassType* backendClass);
}    // namespace Rift::Compiler
