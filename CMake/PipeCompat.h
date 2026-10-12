// Copyright 2015-2026 Piperift. All Rights Reserved.
//
// Compatibility shim for a header-ordering issue in the Pipe submodule.
//
// Pipe's `PipeType.h` references `p::IsTypeParentOf(...)` from a template body,
// but that function is only declared later in `PipeReflect.h` (which itself
// includes `PipeType.h`). Recent Clang/GCC versions reject the non-dependent
// name lookup at parse time. Force-including this header makes the declaration
// visible before `PipeType.h` is parsed.
//
// This file can be removed once the Pipe headers declare `IsTypeParentOf`
// before it is used (or include `PipeReflect.h` before the template).

#pragma once

namespace p
{
	struct TypeId;
	bool IsTypeParentOf(TypeId parentId, TypeId childId);
}    // namespace p
