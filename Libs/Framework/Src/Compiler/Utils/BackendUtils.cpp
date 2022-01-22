// Copyright 2015-2022 Piperift - All rights reserved

#include "Compiler/Utils/BackendUtils.h"


namespace Rift::Compiler
{
	TArray<Refl::ClassType*> GetBackendTypes()
	{
		TArray<Refl::ClassType*> types = Backend::GetStaticType()->GetChildren();
		types.RemoveIf([](Refl::ClassType* type) {
			return !type || type->HasFlag(Class_Abstract);
		});
		return Move(types);
	}

	TArray<TOwnPtr<Backend>> CreateBackends()
	{
		TArray<TOwnPtr<Backend>> backends;

		auto types = GetBackendTypes();
		backends.Reserve(types.Size());
		for (auto* type : types)
		{
			backends.Add(MakeOwned<Backend>(type));
		}
		return Move(backends);
	}
}    // namespace Rift::Compiler
