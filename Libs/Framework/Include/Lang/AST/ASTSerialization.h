// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Lang/AST.h"
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


		void SerializeRoot(AST::Id& root);

		const TArray<AST::Id>& GetASTIds() const
		{
			return ASTIds;
		}
	};


	class ASTWriteContext : public Serl::WriteContext
	{
		const AST::Tree& ast;

		// While serializing we create ids as AST::Ids appear and link them.
		u32 nodeCount = 0;
		TMap<AST::Id, u32> ASTIdToIndexes;


	public:
		ASTWriteContext(const Serl::WriteContext& parent, AST::Tree& ast)
		    : Serl::WriteContext(parent)
		    , ast(ast)
		{}

		void SerializeRoot(const AST::Id& root);

		const TMap<AST::Id, u32>& GetASTIdToIndexes() const
		{
			return ASTIdToIndexes;
		}

	private:
		void RetrieveHierarchy(AST::Id root, TArray<AST::Id>& children);
	};
}    // namespace Rift
