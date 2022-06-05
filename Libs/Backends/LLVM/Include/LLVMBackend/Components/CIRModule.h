// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <llvm/IR/Module.h>
#include <Reflection/Struct.h>


namespace rift
{
	struct CIRModule : public pipe::Struct
	{
		STRUCT(CIRModule, pipe::Struct)

		pipe::TOwnPtr<llvm::Module> instance;
	};
}    // namespace rift
