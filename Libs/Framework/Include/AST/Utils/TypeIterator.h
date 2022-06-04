// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Utils/Paths.h"

#include <Core/Set.h>
#include <Files/FormatFileIterator.h>


namespace Rift
{
	class TypeIterator : public FormatFileIterator<Files::RecursiveIterator>
	{
	public:
		using FormatFileIterator<Files::RecursiveIterator>::FormatFileIterator;

		explicit TypeIterator(const Pipe::Path& path, const TSet<Path>* ignorePaths = nullptr)
		    : FormatFileIterator(Paths::typeExtension, path)
		{}

		const Pipe::Path& operator*() const noexcept
		{
			return fileIterator->path();
		}

		const Pipe::Path* operator->() const noexcept
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
}    // namespace Rift
