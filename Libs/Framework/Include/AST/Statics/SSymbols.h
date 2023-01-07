// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"
#include "AST/Utils/Namespaces.h"

#include <Pipe/Reflect/Struct.h>


namespace rift::AST
{
	using namespace p::core;

	struct SSymbols : public Struct
	{
		STRUCT(SSymbols, Struct)

		// Cached map of symbols
		TMap<Namespace, Id> symbols;
	};
}    // namespace rift::AST
