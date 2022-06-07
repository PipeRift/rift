// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Utils/Paths.h"

#include <Pipe/Core/Set.h>
#include <Pipe/Files/FormatFileIterator.h>


namespace rift
{
	class TypeIterator : public FormatFileIterator<files::RecursiveIterator>
	{
	public:
		using FormatFileIterator<files::RecursiveIterator>::FormatFileIterator;

		explicit TypeIterator(const p::Path& path, const TSet<Path>* ignorePaths = nullptr)
		    : FormatFileIterator(Paths::typeExtension, path)
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
}    // namespace rift
