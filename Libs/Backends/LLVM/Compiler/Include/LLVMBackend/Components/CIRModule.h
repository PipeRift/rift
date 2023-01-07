// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <llvm/IR/Module.h>
#include <Pipe/Reflect/Struct.h>


namespace rift
{
	struct CIRModule : public p::Struct
	{
		STRUCT(CIRModule, p::Struct)

		p::TOwnPtr<llvm::Module> instance;

		p::String objectFile;
	};
}    // namespace rift
