// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <AST/Tree.h>
#include <Files/Paths.h>
#include <Types/Struct.h>


namespace Rift
{
	/**
	 * This component points an AST node to a file.
	 * Some examples are Class, Struct and Function Library declarations pointing to their files
	 */
	struct CFileRef : public Struct
	{
		STRUCT(CFileRef, Struct, Struct_NotSerialized)

		PROP(path)
		Path path;


		CFileRef() {}
		CFileRef(Path path) : path(path) {}
	};
}    // namespace Rift
