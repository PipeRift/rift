// Copyright 2015-2022 Piperift - All rights reserved

#include "Utils/EditorColors.h"


namespace Rift::Style
{
	const Color GetTypeColor(const AST::Tree& ast, AST::Id id)
	{
		const auto& nativeIds = ast.GetNativeTypes();
		if (id == nativeIds.boolId)
		{
			return GetTypeColor<bool>();
		}
		// Floating point
		else if (id == nativeIds.floatId || id == nativeIds.doubleId)
		{
			return GetTypeColor<double>();
		}
		// Unsigned integer
		else if (id == nativeIds.u8Id || id == nativeIds.u16Id || id == nativeIds.u32Id
		         || id == nativeIds.u64Id)
		{
			return GetTypeColor<u32>();
		}
		// Signed integer
		else if (id == nativeIds.i8Id || id == nativeIds.i16Id || id == nativeIds.i32Id
		         || id == nativeIds.i64Id)
		{
			return GetTypeColor<i32>();
		}
		else if (id == nativeIds.stringId)
		{
			return GetTypeColor<String>();
		}
		return GetTypeColor<void>();
	};
}    // namespace Rift::Style
