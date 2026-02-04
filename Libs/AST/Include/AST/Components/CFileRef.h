// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include <AST/Tree.h>
#include <Pipe/Files/Paths.h>
#include <PipeReflect.h>


namespace rift::ast
{
	/**
	 * This component points an AST node to a file.
	 * Some examples are Class, p::Struct and Function Library declarations pointing to their
	 * files
	 */
	struct CFileRef
	{
		P_STRUCT(CFileRef, p::TF_NotSerialized)

		P_PROP(path)
		p::String path;


		CFileRef() {}
		CFileRef(p::StringView path) : path{path} {}
	};
}    // namespace rift::ast
