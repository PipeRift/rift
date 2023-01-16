// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/CModule.h"

#include <Pipe/Core/Set.h>
#include <Pipe/Files/LambdaFileIterator.h>


namespace rift::AST
{
	class ModuleIterator : public p::LambdaFileIterator<p::RecursiveIterator>
	{
		using Super = p::LambdaFileIterator<p::RecursiveIterator>;

	public:
		using Super::Super;

		explicit ModuleIterator(const p::Path& path)
		    : Super(path, [](const auto& path) {
			    return path.filename() == moduleFilename;
		    })
		{}
	};


	inline ModuleIterator begin(ModuleIterator it) noexcept
	{
		return it;
	}

	inline ModuleIterator end(ModuleIterator) noexcept
	{
		return {};
	}
}    // namespace rift::AST
