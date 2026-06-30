// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "AST/Components/CFileRef.h"
#include "AST/Components/Expressions.h"

#include <PipeECS.h>
#include <PipeReflect.h>


namespace rift::ast
{
	// Dirty tags are cleaned manually by the respective systems. CChanged instead gets cleared
	// after one frame
	template<typename T>
	struct TDirty
	{
		P_STRUCT(TDirty<T>, p::TF_NotSerialized)
	};

	using CDirty = p::CMdfd<void>;

	// Marks a type as dirty, meaning is has been modified
	using CFileDirty = p::CMdfd<CFileRef>;

}    // namespace rift::ast
