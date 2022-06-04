// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"
#include "Serialization/Contexts.h"


namespace Rift::AST
{
	using namespace Pipe;


	class ReadContext : public Pipe::ReadContext
	{
		Tree& ast;

		// While serializing we create ids as Ids appear and link them.
		u32 nodeCount = 0;
		TArray<Id> ids;


	public:
		ReadContext(const Pipe::ReadContext& parent, Tree& ast)
		    : Serl::ReadContext(parent), ast(ast)
		{}

		void SerializeEntities(Pipe::TArray<Id>& entities);

		void SerializeEntity(Id& entity)
		{
			TArray<Id> entities{entity};
			SerializeEntities(entities);
			entity = entities.IsEmpty() ? NoId : entities[0];
		}

		const TArray<Id>& GetIds() const
		{
			return ids;
		}
	};


	class WriteContext : public Pipe::WriteContext
	{
		Tree& ast;
		bool includeChildren;

		// While serializing we create ids as Ids appear and link them.
		u32 nodeCount = 0;
		TMap<Id, i32> idToIndexes;


	public:
		WriteContext(const Pipe::WriteContext& parent, Tree& ast, bool includeChildren = true)
		    : Serl::WriteContext(parent), ast(ast), includeChildren{includeChildren}
		{}

		void SerializeEntities(const TArray<Id>& entities, bool includeChildren = true);

		void SerializeEntity(Id entity, bool includeChildren = true)
		{
			SerializeEntities({entity}, includeChildren);
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
