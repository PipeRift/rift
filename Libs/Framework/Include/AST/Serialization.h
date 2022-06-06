// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"
#include "Serialization/Serialization.h"


namespace rift::AST
{
	using namespace p;


	class Reader : public p::Reader
	{
		Tree& ast;

		// While serializing we create ids as Ids appear and link them.
		u32 nodeCount = 0;
		TArray<Id> ids;


	public:
		Reader(const p::Reader& parent, Tree& ast) : p::Reader(parent), ast(ast) {}

		void SerializeEntities(p::TArray<Id>& entities);

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


	class Writer : public p::Writer
	{
		Tree& ast;
		bool includeChildren;

		// While serializing we create ids as Ids appear and link them.
		u32 nodeCount = 0;
		TMap<Id, i32> idToIndexes;


	public:
		Writer(const p::Writer& parent, Tree& ast, bool includeChildren = true)
		    : p::Writer(parent), ast(ast), includeChildren{includeChildren}
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
}    // namespace rift::AST
