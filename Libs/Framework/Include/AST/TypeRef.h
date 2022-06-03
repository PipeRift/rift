// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CType.h"
#include "AST/Tree.h"

#include <Misc/Checks.h>


namespace Rift::AST
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
				Ensure(ast.Has<CType>(typeId));
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
}    // namespace Rift::AST
