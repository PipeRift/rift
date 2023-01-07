// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Utils/Paths.h"

#include <Pipe/Core/Set.h>
#include <Pipe/Files/FormatFileIterator.h>


namespace rift::AST
{
	class TypeIterator : public p::files::FormatFileIterator<p::files::RecursiveIterator>
	{
		using Super = p::files::FormatFileIterator<p::files::RecursiveIterator>;

	public:
		using Super::Super;

		explicit TypeIterator(const p::Path& path, const p::TSet<p::Path>* ignorePaths = nullptr)
		    : Super(Paths::typeExtension, path)
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


	inline TypeIterator begin(TypeIterator it) noexcept
	{
		return it;
	}

	inline TypeIterator end(TypeIterator) noexcept
	{
		return {};
	}
}    // namespace rift::AST
