// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "CoreEngine.h"
#include "CoreTypes.h"


namespace Rift
{
	enum class ReflectionTags : u16
	{
		None = 0,

		// Class only Tags
		Abstract = 1 << 0,

		// Class & Property Tags
		Transient = 1 << 8,
		SaveGame = 1 << 9,

		// Property only Tags
		DetailsEdit = 1 << 12,
		DetailsView = 1 << 13
	};

#define Abstract ReflectionTags::Abstract
#define Transient ReflectionTags::Transient
#define SaveGame ReflectionTags::SaveGame
#define DetailsEdit ReflectionTags::DetailsEdit
#define DetailsView ReflectionTags::DetailsView


	constexpr u16 operator*(ReflectionTags f)
	{
		return static_cast<u16>(f);
	}

	constexpr ReflectionTags operator|(ReflectionTags lhs, ReflectionTags rhs)
	{
		return static_cast<ReflectionTags>((*lhs) | (*rhs));
	}

	constexpr ReflectionTags operator&(ReflectionTags lhs, ReflectionTags rhs)
	{
		return static_cast<ReflectionTags>((*lhs) & (*rhs));
	}

	constexpr bool operator!(ReflectionTags lhs)
	{
		return *lhs <= 0;
	}

	constexpr bool operator>(ReflectionTags lhs, u16 rhs)
	{
		return (*lhs) > rhs;
	}

	constexpr bool operator<(ReflectionTags lhs, u16 rhs)
	{
		return (*lhs) < rhs;
	}

	constexpr bool operator>(u16 lhs, ReflectionTags rhs)
	{
		return lhs > (*rhs);
	}

	constexpr bool operator<(u16 lhs, ReflectionTags rhs)
	{
		return lhs < (*rhs);
	}


	// Allows initialization from macros being <> a value of 0
	template <ReflectionTags tags = static_cast<ReflectionTags>(0)>
	struct ReflectionTagsInitializer
	{
		static constexpr ReflectionTags value = tags;
	};
}	 // namespace Rift
