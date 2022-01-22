// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <Compiler/Backend.h>


namespace Rift::Compiler::Cpp
{
	class CppBackend : public Backend
	{
		CLASS(CppBackend, Backend)

	public:
		Name GetName() override
		{
			return "Cpp";
		}
	};
}    // namespace Rift::Compiler::Cpp
