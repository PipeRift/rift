// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <AST/Tree.h>
#include <Files/Paths.h>
#include <Reflection/Struct.h>


namespace rift
{
	using namespace p;


	/**
	 * This component points an AST node to a file.
	 * Some examples are Class, p::Struct and Function Library declarations pointing to their
	 * files
	 */
	struct CFileRef : public p::Struct
	{
		STRUCT(CFileRef, p::Struct, p::Struct_NotSerialized)

		PROP(path)
		Path path;


		CFileRef() {}
		CFileRef(Path path) : path(path) {}
	};
}    // namespace rift
