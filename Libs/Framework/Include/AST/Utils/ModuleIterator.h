// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Utils/Paths.h"

#include <Core/Set.h>
#include <Files/FormatFileIterator.h>


namespace Rift
{
	class ModuleIterator : public FormatFileIterator<Files::RecursiveIterator>
	{
	public:
		using FormatFileIterator<Files::RecursiveIterator>::FormatFileIterator;

		explicit ModuleIterator(const Pipe::Path& path, const TSet<Path>* ignorePaths = nullptr)
		    : FormatFileIterator(Paths::moduleExtension, path)
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


	inline ModuleIterator begin(ModuleIterator it) noexcept
	{
		return it;
	}

	inline ModuleIterator end(ModuleIterator) noexcept
	{
		return {};
	}
}    // namespace Rift
