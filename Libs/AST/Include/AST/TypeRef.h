// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/CDeclType.h"
#include "AST/Tree.h"

#include <Pipe/Core/Checks.h>


namespace rift::AST
{
	struct TypeRef
	{
	private:
		Tree& ast;
		AST::Id typeId = AST::NoId;

	public:
		TypeRef(Tree& ast, AST::Id typeId) : ast(ast), typeId(typeId)
		{
			if (!IsNone(typeId))
			{
				Ensure(ast.Has<CDeclType>(typeId));
			}
		}

		Tree& GetContext()
		{
			return ast;
		}
		const Tree& GetContext() const
		{
			return ast;
		}

		AST::Id GetId() const
		{
			return typeId;
		}

		bool IsValid() const
		{
			return !IsNone(typeId);
		}


		operator AST::Id() const
		{
			return typeId;
		}
		operator bool() const
		{
			return IsValid();
		}
	};
}    // namespace rift::AST
