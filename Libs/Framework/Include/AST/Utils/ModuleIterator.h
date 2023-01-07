// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Utils/Paths.h"

#include <Pipe/Core/Set.h>
#include <Pipe/Files/FormatFileIterator.h>


namespace rift::AST
{
	class ModuleIterator : public FormatFileIterator<files::RecursiveIterator>
	{
	public:
		using FormatFileIterator<files::RecursiveIterator>::FormatFileIterator;

		explicit ModuleIterator(const p::Path& path, const TSet<Path>* ignorePaths = nullptr)
		    : FormatFileIterator(Paths::moduleExtension, path)
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


	inline ModuleIterator begin(ModuleIterator it) noexcept
	{
		return it;
	}

	inline ModuleIterator end(ModuleIterator) noexcept
	{
		return {};
	}
}    // namespace rift::AST
