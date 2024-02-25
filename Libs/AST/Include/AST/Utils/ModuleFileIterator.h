// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/CModule.h"

#include <Pipe/Core/Set.h>
#include <Pipe/Files/LambdaFileIterator.h>


namespace rift::ast
{
	class ModuleFileIterator : public p::LambdaFileIterator<p::RecursivePathIterator>
	{
		using Super = p::LambdaFileIterator<p::RecursivePathIterator>;

	public:
		using Super::Super;

		explicit ModuleFileIterator(p::StringView path)
		    : Super(path, [](p::StringView path) {
			    return p::GetFilename(path) == moduleFilename;
		    })
		{}
	};


	inline ModuleFileIterator begin(ModuleFileIterator it) noexcept
	{
		return it;
	}

	inline ModuleFileIterator end(ModuleFileIterator) noexcept
	{
		return {};
	}
}    // namespace rift::ast
