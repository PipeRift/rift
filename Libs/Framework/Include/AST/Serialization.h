// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"
#include "Serialization/Contexts.h"


namespace Rift
{
	class ASTReadContext : public Serl::ReadContext
	{
		AST::Tree& ast;

		// While serializing we create ids as AST::Ids appear and link them.
		u32 nodeCount = 0;
		TArray<AST::Id> ASTIds;


	public:
		ASTReadContext(const Serl::ReadContext& parent, AST::Tree& ast)
		    : Serl::ReadContext(parent)
		    , ast(ast)
		{}

		void SerializeRoots(TArray<AST::Id>& roots);

		void SerializeRoot(AST::Id& root)
		{
			TArray<AST::Id> roots{root};
			SerializeRoots(roots);
			root = roots.IsEmpty() ? AST::NoId : roots[0];
		}

		const TArray<AST::Id>& GetASTIds() const
		{
			return ASTIds;
		}
	};


	class ASTWriteContext : public Serl::WriteContext
	{
		AST::Tree& ast;
		bool includeChildren;

		// While serializing we create ids as AST::Ids appear and link them.
		u32 nodeCount = 0;
		TMap<AST::Id, u32> ASTIdToIndexes;


	public:
		ASTWriteContext(
		    const Serl::WriteContext& parent, AST::Tree& ast, bool includeChildren = true)
		    : Serl::WriteContext(parent)
		    , ast(ast)
		    , includeChildren{includeChildren}
		{}

		void SerializeRoots(const TArray<AST::Id>& roots);

		void SerializeRoot(const AST::Id& root)
		{
			TArray<AST::Id> roots{root};
			SerializeRoots(roots);
		}

		const TMap<AST::Id, u32>& GetASTIdToIndexes() const
		{
			return ASTIdToIndexes;
		}

	private:
		void RetrieveHierarchy(const TArray<AST::Id>& roots, TArray<AST::Id>& children);
	};
}    // namespace Rift
