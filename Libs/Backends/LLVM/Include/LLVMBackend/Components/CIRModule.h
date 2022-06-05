// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <llvm/IR/Module.h>
#include <Reflection/Struct.h>


namespace rift
{
	struct CIRModule : public p::Struct
	{
		STRUCT(CIRModule, p::Struct)

		p::TOwnPtr<llvm::Module> instance;
	};
}    // namespace rift
