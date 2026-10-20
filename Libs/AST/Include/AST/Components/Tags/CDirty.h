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
	using CDirty = p::CMdfd<void>;

	// Marks a type as dirty, meaning is has been modified
	using CFileDirty = p::CMdfd<CFileRef>;

}    // namespace rift::ast
