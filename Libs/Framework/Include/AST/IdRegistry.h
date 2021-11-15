// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <Containers/Array.h>
#include <Containers/ArrayView.h>


namespace Rift::AST
{
	struct IdRegistry
	{
		using Traits  = IdTraits<Id>;
		using Index   = Traits::Index;
		using Version = Traits::Version;


		Id Create();
		void Create(TArrayView<Id> newIds);
		bool Destroy(Id id);
		bool Destroy(TArrayView<const Id> ids);
		bool IsValid(Id id) const;

		u32 Size() const
		{
			return entities.Size() - available.Size();
		}


	private:

		TArray<Version> entities;
		TArray<Index> available;
	};
}    // namespace Rift::AST
