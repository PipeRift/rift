// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <Compiler/Backend.h>


namespace Rift::Compiler::LLVM
{
	class LLVMBackend : public Backend
	{
		CLASS(LLVMBackend, Backend)

	public:
		Name GetName() override
		{
			return "LLVM";
		}
	};
}    // namespace Rift::Compiler::LLVM
