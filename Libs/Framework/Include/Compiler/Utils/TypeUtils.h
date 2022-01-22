// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Compiler/Context.h"

#include <Memory/OwnPtr.h>
#include <Refl/Static/ClassType.h>


namespace Rift::Compiler
{
	TArray<Refl::ClassType*> FindBackends();

	TOwnPtr<Context> CreateBackend(Refl::ClassType* backendClass);
}    // namespace Rift::Compiler
