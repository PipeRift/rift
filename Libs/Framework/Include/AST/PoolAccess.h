// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <Containers/Array.h>
#include <Reflection/TypeId.h>
#include <Templates/TypeList.h>


namespace Rift::AST
{
	template<typename... T>
	struct TAccess;

	template<typename... IncludeComp, typename... ExcludeComp>
	struct TAccess<TInclude<IncludeComp...>, TExclude<ExcludeComp...>>
	{
		using Included = TTypeList<IncludeComp...>;
		using Excluded = TTypeList<ExcludeComp...>;


		template<typename... T>
		using Include = TAccess<TInclude<IncludeComp..., T...>, TExclude<ExcludeComp...>>;

		template<typename... T>
		using Exclude = TAccess<TInclude<ExcludeComp...>, TExclude<ExcludeComp..., T...>>;
	};

	struct Access
	{
		template<typename... T>
		using Include = TAccess<TInclude<T...>, TExclude<>>;
	};


	struct PoolAccess
	{
	protected:
		TArray<Refl::TypeId> included;
		TArray<Refl::TypeId> excluded;


	public:
		PoolAccess() {}

		i32 Include(Refl::TypeId id)
		{
			return included.FindOrAddSorted(id).first;
		}
		i32 Exclude(Refl::TypeId id)
		{
			return excluded.FindOrAddSorted(id).first;
		}

		bool RemoveInclude(Refl::TypeId id)
		{
			return included.RemoveSorted(id) > 0;
		}
		bool RemoveExclude(Refl::TypeId id)
		{
			return excluded.RemoveSorted(id) > 0;
		}

		bool IsIncluded(Refl::TypeId id)
		{
			return included.FindSortedEqual(id) != NO_INDEX;
		}
		bool IsExcluded(Refl::TypeId id)
		{
			return excluded.FindSortedEqual(id) != NO_INDEX;
		}
	};
}    // namespace Rift::AST
