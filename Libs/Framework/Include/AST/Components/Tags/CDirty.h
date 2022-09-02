// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CExprCall.h"
#include "AST/Components/CFileRef.h"

#include <Pipe/Reflect/Struct.h>


namespace rift::AST
{
	// Dirty tags are cleaned manually by the respective systems. CChanged instead gets cleared
	// after one frame
	template<typename T>
	struct TDirty : public p::Struct
	{
		STRUCT(TDirty<T>, p::Struct, p::Struct_NotSerialized)
	};

	using CDirty = TDirty<void>;

	// Marks a type as dirty, meaning is has been modified
	using CFileDirty = TDirty<CFileRef>;

	// Marks a type as dirty, meaning is has been modified
	using CCallDirty = TDirty<CExprCallId>;
}    // namespace rift::AST
