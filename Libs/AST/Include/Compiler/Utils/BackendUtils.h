// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "Compiler/Backend.h"
#include "Compiler/Compiler.h"

#include <Pipe/Memory/OwnPtr.h>
#include <PipeReflect.h>


namespace rift
{
	p::TArray<p::TypeId> GetBackendTypes();
	p::TArray<p::TOwnPtr<Backend>> CreateBackends();
}    // namespace rift
