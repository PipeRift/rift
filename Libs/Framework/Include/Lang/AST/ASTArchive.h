// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Lang/AST.h"


namespace Rift
{
	/*class ASTArchive : public NestedArchive
	{
		AST::AbstractSyntaxTree& ast;

		// While serializing we create ids as AST::Ids appear and link them.
		u32 nodeCount = 0;
		TMap<AST::Id, u32> ASTToDataIds;    // Only available when Saving
		TMap<u32, AST::Id> DataToASTIds;    // Only available when Loading


	public:
		ASTArchive(AST::AbstractSyntaxTree& ast, Archive& inParent)
		    : ast(ast)
		    , NestedArchive(inParent)
		{}


		void SerializeRoot(AST::Id& root);

		// virtual void Serialize(StringView name, AST::Id& val);

	private:
		void RetrieveHierarchy(AST::Id root, TArray<AST::Id>& children);
	};*/
}    // namespace Rift
