// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Compiler/Utils/BackendUtils.h"

#include "PipeReflect.h"


namespace rift
{
	p::TArray<p::TypeId> GetBackendTypes()
	{
		p::TArray<p::TypeId> types;
		const p::TypeId backendType = p::GetTypeId<Backend>();
		for (p::TypeId type : p::GetRegisteredTypeIds())
		{
			if (!type.IsValid() || p::HasTypeFlags(type, p::TF_Abstract))
			{
				continue;
			}
			if (type == backendType || p::IsTypeParentOf(backendType, type))
			{
				types.Add(type);
			}
		}
		return Move(types);
	}

	p::TArray<p::TOwnPtr<Backend>> CreateBackends()
	{
		p::TArray<p::TOwnPtr<Backend>> backends;
		auto types = GetBackendTypes();
		backends.Reserve(types.Size());
		for (p::TypeId type : types)
		{
			backends.Add(p::MakeOwned<Backend>(type));
		}
		return Move(backends);
	}
}    // namespace rift
