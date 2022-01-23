// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <Compiler/Backend.h>


namespace Rift::Compiler
{
	class LLVMBackend : public Backend
	{
		CLASS(LLVMBackend, Backend)

	public:
		Name GetName() override
		{
			return "LLVM";
		}

		void Build(Context& context) override
		{
			context.AddError("LLVM backend is not yet supported.");
		}
	};
}    // namespace Rift::Compiler
