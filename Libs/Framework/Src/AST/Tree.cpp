// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Tree.h"

#include "AST/Components/CDeclNative.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CType.h"
#include "AST/Statics/SModules.h"
#include "AST/Statics/STypes.h"



namespace Rift::AST
{
	Tree::Tree()
	{
		SetupNativeTypes();
		CachePools();
	}

	Tree::Tree(const Tree& other) noexcept
	{
		CopyFrom(other);
	}
	Tree::Tree(Tree&& other) noexcept
	{
		MoveFrom(Move(other));
	}
	Tree& Tree::operator=(const Tree& other) noexcept
	{
		Reset();
		CopyFrom(other);
		return *this;
	}
	Tree& Tree::operator=(Tree&& other) noexcept
	{
		Reset();
		MoveFrom(Move(other));
		return *this;
	}

	Id Tree::Create()
	{
		return idRegistry.Create();
	}
	void Tree::Create(TSpan<Id> ids)
	{
		idRegistry.Create(ids);
	}

	void Tree::Destroy(const Id id)
	{
		idRegistry.Destroy(id);
		for (auto& pool : pools)
		{
			pool.GetPool()->Remove(id);
		}
	}

	void Tree::Destroy(TSpan<const Id> ids)
	{
		idRegistry.Destroy(ids);
		for (auto& pool : pools)
		{
			pool.GetPool()->Remove(ids);
		}
	}

	Pool* Tree::GetPool(Refl::TypeId componentId) const
	{
		const i32 index = pools.FindSortedEqual(PoolInstance{componentId});
		return index != NO_INDEX ? pools[index].GetPool() : nullptr;
	}

	void Tree::SetupNativeTypes()
	{
		// Remove any previous native types
		Destroy(Filter<CDeclNative>().GetIds());

		nativeTypes.boolId = Create();
		Add<CDeclNative>(nativeTypes.boolId);
		Add<CType>(nativeTypes.boolId, {"Bool"});

		nativeTypes.floatId = Create();
		Add<CDeclNative>(nativeTypes.floatId);
		Add<CType>(nativeTypes.floatId, {"Float"});

		nativeTypes.doubleId = Create();
		Add<CDeclNative>(nativeTypes.doubleId);
		Add<CType>(nativeTypes.doubleId, {"Double"});

		nativeTypes.u8Id = Create();
		Add<CDeclNative>(nativeTypes.u8Id);
		Add<CType>(nativeTypes.u8Id, {"U8"});

		nativeTypes.i8Id = Create();
		Add<CDeclNative>(nativeTypes.i8Id);
		Add<CType>(nativeTypes.i8Id, {"I8"});

		nativeTypes.u16Id = Create();
		Add<CDeclNative>(nativeTypes.u16Id);
		Add<CType>(nativeTypes.u16Id, {"U16"});

		nativeTypes.i16Id = Create();
		Add<CDeclNative>(nativeTypes.i16Id);
		Add<CType>(nativeTypes.i16Id, {"I16"});

		nativeTypes.u32Id = Create();
		Add<CDeclNative>(nativeTypes.u32Id);
		Add<CType>(nativeTypes.u32Id, {"U32"});

		nativeTypes.i32Id = Create();
		Add<CDeclNative>(nativeTypes.i32Id);
		Add<CType>(nativeTypes.i32Id, {"I32"});

		nativeTypes.u64Id = Create();
		Add<CDeclNative>(nativeTypes.u64Id);
		Add<CType>(nativeTypes.u64Id, {"U64"});

		nativeTypes.i64Id = Create();
		Add<CDeclNative>(nativeTypes.i64Id);
		Add<CType>(nativeTypes.i64Id, {"I64"});

		nativeTypes.stringId = Create();
		Add<CDeclNative>(nativeTypes.stringId);
		Add<CType>(nativeTypes.stringId, {"String"});
	}

	void Tree::CachePools()
	{
		AssurePool<CParent>();
		AssurePool<CChild>();

		parentView = MakeOwned<TFilter<FilterAccess::In<CParent>>>(Filter<CParent>());
		childView  = MakeOwned<TFilter<FilterAccess::In<CChild>>>(Filter<CChild>());
	}

	void Tree::CopyFrom(const Tree& other)
	{
		// Copy entities
		idRegistry = other.idRegistry;

		// Copy component pools. Assume already sorted
		for (const PoolInstance& otherInstance : other.pools)
		{
			PoolInstance instance{otherInstance};
			instance.pool->TransferToTree(*this);
			pools.Add(Move(instance));
		}

		// Copy non-transient unique components
		// TODO: Use reflection for this
		if (auto* modulesStatic = other.TryGetStatic<SModules>())
		{
			SetStatic<SModules>(*modulesStatic);
		}
		if (auto* typesStatic = other.TryGetStatic<STypes>())
		{
			SetStatic<STypes>(*typesStatic);
		}

		nativeTypes = other.nativeTypes;
		CachePools();
	}

	void Tree::MoveFrom(Tree&& other)
	{
		idRegistry = Move(other.idRegistry);
		pools      = Move(other.pools);
		for (auto& instance : pools)
		{
			instance.pool->TransferToTree(*this);
		}
		statics     = Move(other.statics);
		nativeTypes = other.nativeTypes;

		CachePools();
	}
}    // namespace Rift::AST
