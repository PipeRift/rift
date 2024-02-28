// Copyright 2015-2023 Piperift - All rights reserved

#include "Compiler/Utils/BackendUtils.h"


namespace rift
{
	p::TArray<p::ClassType*> GetBackendTypes()
	{
		p::TArray<p::ClassType*> types = Backend::GetStaticType()->GetChildren();
		types.RemoveIf([](p::ClassType* type) {
			return !type || type->HasFlag(p::Class_Abstract);
		});
		return Move(types);
	}

	p::TArray<p::TOwnPtr<Backend>> CreateBackends()
	{
		p::TArray<p::TOwnPtr<Backend>> backends;
		auto types = GetBackendTypes();
		backends.Reserve(types.Size());
		for (auto* type : types)
		{
			backends.Add(p::MakeOwned<Backend>(type));
		}
		return Move(backends);
	}
}    // namespace rift
