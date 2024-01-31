// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Utils/Paths.h"

#include <Pipe/Core/Set.h>
#include <Pipe/Files/LambdaFileIterator.h>


namespace rift::ast
{
	class TypeIterator : public p::files::LambdaFileIterator<p::files::RecursiveIterator>
	{
		using Super = p::files::LambdaFileIterator<p::files::RecursiveIterator>;

	public:
		using Super::Super;

		explicit TypeIterator(const p::Path& path)
		    : Super(path, [](const auto& path) {
			    return path.extension() == Paths::typeExtension;
		    })
		{}
	};


	inline TypeIterator begin(TypeIterator it) noexcept
	{
		return it;
	}

	inline TypeIterator end(TypeIterator) noexcept
	{
		return {};
	}
}    // namespace rift::ast
