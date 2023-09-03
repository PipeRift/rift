// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <AST/Tree.h>
#include <Pipe/Files/Paths.h>
#include <Pipe/Reflect/Struct.h>


namespace rift::AST
{
	/**
	 * This component points an AST node to a file.
	 * Some examples are Class, p::Struct and Function Library declarations pointing to their
	 * files
	 */
	struct CFileRef : public p::Struct
	{
		P_STRUCT(CFileRef, p::Struct, p::Struct_NotSerialized)

		P_PROP(path)
		p::String path;


		CFileRef() {}
		CFileRef(p::StringView path) : path{path} {}
	};
}    // namespace rift::AST
