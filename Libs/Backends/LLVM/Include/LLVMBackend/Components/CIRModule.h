// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <llvm/IR/Module.h>
#include <Reflection/Struct.h>


namespace Rift
{
	struct CIRModule : public Pipe::Struct
	{
		STRUCT(CIRModule, Pipe::Struct)

		Pipe::TOwnPtr<llvm::Module> instance;
	};
}    // namespace Rift
