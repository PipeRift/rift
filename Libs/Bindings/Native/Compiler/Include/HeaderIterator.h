// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Utils/Paths.h"

#include <Pipe/Core/Set.h>
#include <Pipe/Files/FormatFileIterator.h>


namespace rift
{
	class HeaderIterator : public p::files::FormatFileIterator<p::files::RecursiveIterator>
	{
		using Super = p::files::FormatFileIterator<p::files::RecursiveIterator>;

	public:
		static constexpr p::StringView headerExtension = ".h";
		using Super::Super;

		explicit HeaderIterator(const p::Path& path, const p::TSet<p::Path>* ignorePaths = nullptr)
		    : Super(headerExtension, path)
		{}

		const p::Path& operator*() const noexcept
		{
			return fileIterator->path();
		}

		const p::Path* operator->() const noexcept
		{
			return &operator*();
		}
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
