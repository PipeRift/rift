// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Tree.h"

#include "AST/Components/CIdentifier.h"
#include "AST/Components/CNativeDecl.h"
#include "AST/Components/CType.h"
#include "AST/Uniques/CModulesUnique.h"
#include "AST/Uniques/CTypesUnique.h"
#include "Compiler/Cpp/Components/CCppNativeName.h"


namespace Rift::AST
{
	Tree::Tree()
	{
		SetupNativeTypes();
		CachePools();
	}

	Tree::Tree(Tree&& other) noexcept
	{
		idRegistry = Move(other.idRegistry);
		pools      = Move(other.pools);
		CachePools();
	}
	Tree& Tree::operator=(Tree&& other) noexcept
	{
		Reset();
		idRegistry = Move(other.idRegistry);
		pools      = Move(other.pools);
		CachePools();
		return *this;
	}

	Id Tree::Create()
	{
		return idRegistry.Create();
	}
	void Tree::Create(TArrayView<Id> ids)
	{
		idRegistry.Create(ids);
	}

	void Tree::Destroy(const Id id)
	{
		idRegistry.Destroy(id);
	}

	void Tree::Destroy(TArrayView<const Id> ids)
	{
		idRegistry.Destroy(ids);
	}

	Pool* Tree::FindPool(Refl::TypeId componentId) const
	{
		const i32 index = pools.FindSortedEqual(PoolInstance{componentId});
		return index != NO_INDEX ? pools[index].instance : nullptr;
	}

	void Tree::SetupNativeTypes()
	{
		nativeTypes.boolId = Create();
		Add<CNativeDecl, CType>(nativeTypes.boolId);
		Add<CIdentifier>(nativeTypes.boolId, "Bool");
		Add<CCppNativeName>(nativeTypes.boolId, "bool");

		nativeTypes.floatId = Create();
		Add<CNativeDecl, CType>(nativeTypes.floatId);
		Add<CIdentifier>(nativeTypes.floatId, "Float");
		Add<CCppNativeName>(nativeTypes.floatId, "float");

		nativeTypes.doubleId = Create();
		Add<CNativeDecl, CType>(nativeTypes.doubleId);
		Add<CIdentifier>(nativeTypes.doubleId, "Double");
		Add<CCppNativeName>(nativeTypes.doubleId, "double");

		nativeTypes.u8Id = Create();
		Add<CNativeDecl, CType>(nativeTypes.u8Id);
		Add<CIdentifier>(nativeTypes.u8Id, "U8");
		Add<CCppNativeName>(nativeTypes.u8Id, "std::uint_8");

		nativeTypes.i8Id = Create();
		Add<CNativeDecl, CType>(nativeTypes.i8Id);
		Add<CIdentifier>(nativeTypes.i8Id, "I8");
		Add<CCppNativeName>(nativeTypes.i8Id, "std::int_8");

		nativeTypes.u16Id = Create();
		Add<CNativeDecl, CType>(nativeTypes.u16Id);
		Add<CIdentifier>(nativeTypes.u16Id, "U16");
		Add<CCppNativeName>(nativeTypes.u16Id, "std::uint_16");

		nativeTypes.i16Id = Create();
		Add<CNativeDecl, CType>(nativeTypes.i16Id);
		Add<CIdentifier>(nativeTypes.i16Id, "I16");
		Add<CCppNativeName>(nativeTypes.i16Id, "std::int_16");

		nativeTypes.u32Id = Create();
		Add<CNativeDecl, CType>(nativeTypes.u32Id);
		Add<CIdentifier>(nativeTypes.u32Id, "U32");
		Add<CCppNativeName>(nativeTypes.u32Id, "std::uint_32");

		nativeTypes.i32Id = Create();
		Add<CNativeDecl, CType>(nativeTypes.i32Id);
		Add<CIdentifier>(nativeTypes.i32Id, "I32");
		Add<CCppNativeName>(nativeTypes.i32Id, "std::int_32");

		nativeTypes.u64Id = Create();
		Add<CNativeDecl, CType>(nativeTypes.u64Id);
		Add<CIdentifier>(nativeTypes.u64Id, "U64");
		Add<CCppNativeName>(nativeTypes.u64Id, "std::uint_64");

		nativeTypes.i64Id = Create();
		Add<CNativeDecl, CType>(nativeTypes.i64Id);
		Add<CIdentifier>(nativeTypes.i64Id, "I64");
		Add<CCppNativeName>(nativeTypes.i64Id, "std::int_64");

		nativeTypes.stringId = Create();
		Add<CNativeDecl, CType>(nativeTypes.stringId);
		Add<CIdentifier>(nativeTypes.stringId, "String");
		Add<CCppNativeName>(nativeTypes.stringId, "std::string");
	}

	void Tree::CachePools()
	{
		AssurePool<CChild>();
		AssurePool<CParent>();

		childView  = MakeOwned<TQuery<TExclude<>, CChild>>(Query<CChild>());
		parentView = MakeOwned<TQuery<TExclude<>, CParent>>(Query<CParent>());
	}

	void Tree::CopyFrom(const Tree& other)
	{
		Reset();

		// Copy entities
		idRegistry = other.idRegistry;

		// Copy components
		pools.Empty();
		for (const PoolInstance& pool : other.pools)
		{
			// pool.Clone(*this);
		}

		// Copy non-transient unique components
		// TODO: Use reflection for this
		SetStatic<CModulesUnique>(other.GetStatic<CModulesUnique>());
		SetStatic<CTypesUnique>(other.GetStatic<CTypesUnique>());

		CachePools();
	}
}    // namespace Rift::AST
