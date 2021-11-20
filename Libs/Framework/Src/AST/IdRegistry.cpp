// Copyright 2015-2021 Piperift - All rights reserved

#include "AST/IdRegistry.h"


namespace Rift::AST
{
	Id IdRegistry::Create()
	{
		if (!available.IsEmpty())
		{
			const Index index = available.Last();
			available.RemoveLast();
			return entities[index];
		}

		const Id id = Traits::Make(entities.Size(), 0);
		entities.Add(id);
		return id;
	}

	void IdRegistry::Create(TArrayView<Id> newIds)
	{
		const i32 availablesUsed = Math::Min(newIds.Size(), available.Size());
		for (i32 i = 0; i < availablesUsed; ++i)
		{
			const Index index = available.Last();
			newIds[i]         = entities[index];
		}
		available.RemoveLast(availablesUsed);

		// Remaining entities
		const u32 remainingSize = newIds.Size() - availablesUsed;
		entities.Reserve(entities.Size() + remainingSize);
		for (i32 i = availablesUsed; i < newIds.Size(); ++i)
		{
			const Id id = Traits::Make(entities.Size(), 0);
			entities.Add(id);
			newIds[i] = id;
		}
	}

	bool IdRegistry::Destroy(Id id)
	{
		const Index index = Traits::GetIndex(id);
		if (entities.IsValidIndex(index))
		{
			Id& storedId = entities[index];
			if (id == storedId)
			{
				// Increase version to invalidate current entity
				storedId = Traits::Make(index, Traits::GetVersion(storedId) + 1u);
				available.Add(index);
				return true;
			}
		}
		return false;
	}

	bool IdRegistry::Destroy(TArrayView<const Id> ids)
	{
		available.Reserve(available.Size() + ids.Size());
		const u32 lastAvailable = available.Size();
		for (AST::Id id : ids)
		{
			const Index index = Traits::GetIndex(id);
			if (entities.IsValidIndex(index))
			{
				Id& storedId = entities[index];
				if (id == storedId)
				{
					// Increase version to invalidate current entity
					storedId = Traits::Make(index, Traits::GetVersion(storedId) + 1u);
					available.Add(index);
				}
			}
		}
		return (available.Size() - lastAvailable) > 0;
	}

	bool IdRegistry::IsValid(Id id) const
	{
		const Index index = Traits::GetIndex(id);
		return entities.IsValidIndex(index) && entities[index] == id;
	}
}    // namespace Rift::AST
