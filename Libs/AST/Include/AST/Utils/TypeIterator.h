// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Utils/Paths.h"

#include <Pipe/Core/Set.h>
#include <Pipe/Files/LambdaFileIterator.h>


namespace rift::ast
{
	class TypeIterator : public p::LambdaFileIterator<p::RecursiveIterator>
	{
		using Super = p::LambdaFileIterator<p::RecursiveIterator>;

	public:
		using Super::Super;

		explicit TypeIterator(p::StringView path)
		    : Super(path, [](p::StringView path) {
			    return p::GetExtension(path) == Paths::typeExtension;
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
