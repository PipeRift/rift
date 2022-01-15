// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Components/CCallExpr.h"
#include "AST/Components/CType.h"

#include <Object/Struct.h>


namespace Rift
{
	// Dirty tags are cleaned manually by the respective systems. CChanged instead gets cleared
	// after one frame
	template<typename T>
	struct TDirty : public Struct
	{
		STRUCT(TDirty<T>, Struct, Struct_NotSerialized)
	};

	using CDirty = TDirty<void>;

	// Marks a type as dirty, meaning is has been modified
	using CTypeDirty = TDirty<CType>;

	// Marks a type as dirty, meaning is has been modified
	using CCallDirty = TDirty<CCallExprId>;
}    // namespace Rift
