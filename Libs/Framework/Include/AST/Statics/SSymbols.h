// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"
#include "AST/Utils/Namespaces.h"

#include <Pipe/Reflect/Struct.h>


namespace rift
{
	struct SSymbols : public p::Struct
	{
		STRUCT(SSymbols, p::Struct)

		// Cached map of symbols
		TMap<AST::Namespace, AST::Id> symbols;
	};
}    // namespace rift
