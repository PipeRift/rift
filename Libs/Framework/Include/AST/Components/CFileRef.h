// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <AST/Tree.h>
#include <Files/Paths.h>
#include <Reflection/Struct.h>


namespace Rift
{
	using namespace Pipe;


	/**
	 * This component points an AST node to a file.
	 * Some examples are Class, Pipe::Struct and Function Library declarations pointing to their
	 * files
	 */
	struct CFileRef : public Pipe::Struct
	{
		STRUCT(CFileRef, Pipe::Struct, Pipe::Struct_NotSerialized)

		PROP(path)
		Path path;


		CFileRef() {}
		CFileRef(Path path) : path(path) {}
	};
}    // namespace Rift
