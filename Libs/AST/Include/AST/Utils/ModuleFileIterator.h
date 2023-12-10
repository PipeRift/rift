// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/CModule.h"

#include <Pipe/Core/Set.h>
#include <Pipe/Files/LambdaFileIterator.h>


namespace rift::AST
{
	class ModuleFileIterator : public p::LambdaFileIterator<p::RecursiveIterator>
	{
		using Super = p::LambdaFileIterator<p::RecursiveIterator>;

	public:
		using Super::Super;

		explicit ModuleFileIterator(const p::Path& path)
		    : Super(path, [](const auto& path) {
			    return path.filename() == moduleFilename;
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
}    // namespace rift::AST