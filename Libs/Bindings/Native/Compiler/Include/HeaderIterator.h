// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Utils/Paths.h"

#include <Pipe/Core/Set.h>
#include <Pipe/Files/LambdaFileIterator.h>


namespace rift
{
	class HeaderIterator : public p::LambdaFileIterator<p::files::RecursiveIterator>
	{
		using Super = p::LambdaFileIterator<p::files::RecursiveIterator>;

	public:
		using Super::Super;

		static constexpr p::StringView headerExtension = ".h";

		explicit HeaderIterator(StringView path)
		    : Super(path, [](StringView path) {
			    return p::GetExtension(path) == headerExtension;
		    })
		{}
	};


	inline HeaderIterator begin(HeaderIterator it) noexcept
	{
		return it;
	}

	inline HeaderIterator end(HeaderIterator) noexcept
	{
		return {};
	}
}    // namespace rift
