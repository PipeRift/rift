// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"
#include "Serialization/Contexts.h"


namespace Rift::AST
{
	class ReadContext : public Serl::ReadContext
	{
		Tree& ast;

		// While serializing we create ids as Ids appear and link them.
		u32 nodeCount = 0;
		TArray<Id> ids;


	public:
		ReadContext(const Serl::ReadContext& parent, Tree& ast)
		    : Serl::ReadContext(parent), ast(ast)
		{}

		void SerializeRoots(TArray<Id>& roots);

		void SerializeRoot(Id& root)
		{
			TArray<Id> roots{root};
			SerializeRoots(roots);
			root = roots.IsEmpty() ? NoId : roots[0];
		}

		const TArray<Id>& GetIds() const
		{
			return ids;
		}
	};


	class WriteContext : public Serl::WriteContext
	{
		Tree& ast;
		bool includeChildren;

		// While serializing we create ids as Ids appear and link them.
		u32 nodeCount = 0;
		TMap<Id, i32> idToIndexes;


	public:
		WriteContext(const Serl::WriteContext& parent, Tree& ast, bool includeChildren = true)
		    : Serl::WriteContext(parent), ast(ast), includeChildren{includeChildren}
		{}

		void SerializeRoots(const TArray<Id>& roots);

		void SerializeRoot(const Id& root)
		{
			TArray<Id> roots{root};
			SerializeRoots(roots);
		}

		const TMap<Id, i32>& GetIdToIndexes() const
		{
			return idToIndexes;
		}

	private:
		void RetrieveHierarchy(const TArray<Id>& roots, TArray<Id>& children);
		void RemoveIgnoredEntities(TArray<Id>& entities);
	};
}    // namespace Rift::AST
