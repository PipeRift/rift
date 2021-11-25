// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <Containers/Array.h>
#include <Containers/Span.h>


namespace Rift::AST
{
	struct IdRegistry
	{
		using Traits  = IdTraits<Id>;
		using Index   = Traits::Index;
		using Version = Traits::Version;

		IdRegistry()                        = default;
		IdRegistry(IdRegistry&& other)      = default;
		IdRegistry(const IdRegistry& other) = default;
		IdRegistry& operator=(IdRegistry&& other) = default;
		IdRegistry& operator=(const IdRegistry& other) = default;


		Id Create();
		void Create(TSpan<Id> newIds);
		bool Destroy(Id id);
		bool Destroy(TSpan<const Id> ids);
		bool IsValid(Id id) const;

		u32 Size() const
		{
			return entities.Size() - available.Size();
		}

		template<typename Callback>
		void Each(Callback cb) const
		{
			if (available.IsEmpty())
			{
				for (i32 i = entities.Size(); i; ++i)
				{
					cb(entities[i]);
				}
			}
			else
			{
				for (i32 i = 0; i < entities.Size(); ++i)
				{
					const Id id = entities[i];
					if (GetIndex(id) == i)
					{
						cb(id);
					}
				}
			}
		}

	private:

		TArray<Id> entities;
		TArray<Index> available;
	};
}    // namespace Rift::AST
