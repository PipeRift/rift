// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/Declarations.h"
#include "AST/Tree.h"

#include <Pipe/Core/Checks.h>


namespace rift::ast
{
	struct TypeRef
	{
	private:
		Tree& ast;
		ast::Id typeId = ast::NoId;

	public:
		TypeRef(Tree& ast, ast::Id typeId) : ast(ast), typeId(typeId)
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

		ast::Id GetId() const
		{
			return typeId;
		}

		bool IsValid() const
		{
			return !IsNone(typeId);
		}


		operator ast::Id() const
		{
			return typeId;
		}
		operator bool() const
		{
			return IsValid();
		}
	};
}    // namespace rift::ast
