// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Tree.h"

#include "AST/Components/CIdentifier.h"
#include "AST/Components/CNativeDecl.h"
#include "AST/Components/CType.h"
#include "AST/Statics/SModules.h"
#include "AST/Statics/STypes.h"
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
		statics    = Move(other.statics);
		CachePools();
	}
	Tree& Tree::operator=(Tree&& other) noexcept
	{
		Reset();
		idRegistry = Move(other.idRegistry);
		pools      = Move(other.pools);
		statics    = Move(other.statics);
		CachePools();
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
			pool.GetInstance()->Remove(id);
		}
	}

	void Tree::Destroy(TSpan<const Id> ids)
	{
		idRegistry.Destroy(ids);
		for (auto& pool : pools)
		{
			pool.GetInstance()->Remove(ids);
		}
	}

	Pool* Tree::FindPool(Refl::TypeId componentId) const
	{
		const i32 index = pools.FindSortedEqual(PoolInstance{componentId});
		return index != NO_INDEX ? pools[index].GetInstance() : nullptr;
	}

	void Tree::SetupNativeTypes()
	{
		nativeTypes.boolId = Create();
		Add<CNativeDecl, CType>(nativeTypes.boolId);
		Add<CIdentifier>(nativeTypes.boolId, {"Bool"});
		Add<CCppNativeName>(nativeTypes.boolId, {"bool"});

		nativeTypes.floatId = Create();
		Add<CNativeDecl, CType>(nativeTypes.floatId);
		Add<CIdentifier>(nativeTypes.floatId, {"Float"});
		Add<CCppNativeName>(nativeTypes.floatId, {"float"});

		nativeTypes.doubleId = Create();
		Add<CNativeDecl, CType>(nativeTypes.doubleId);
		Add<CIdentifier>(nativeTypes.doubleId, {"Double"});
		Add<CCppNativeName>(nativeTypes.doubleId, {"double"});

		nativeTypes.u8Id = Create();
		Add<CNativeDecl, CType>(nativeTypes.u8Id);
		Add<CIdentifier>(nativeTypes.u8Id, {"U8"});
		Add<CCppNativeName>(nativeTypes.u8Id, {"std::uint_8"});

		nativeTypes.i8Id = Create();
		Add<CNativeDecl, CType>(nativeTypes.i8Id);
		Add<CIdentifier>(nativeTypes.i8Id, {"I8"});
		Add<CCppNativeName>(nativeTypes.i8Id, {"std::int_8"});

		nativeTypes.u16Id = Create();
		Add<CNativeDecl, CType>(nativeTypes.u16Id);
		Add<CIdentifier>(nativeTypes.u16Id, {"U16"});
		Add<CCppNativeName>(nativeTypes.u16Id, {"std::uint_16"});

		nativeTypes.i16Id = Create();
		Add<CNativeDecl, CType>(nativeTypes.i16Id);
		Add<CIdentifier>(nativeTypes.i16Id, {"I16"});
		Add<CCppNativeName>(nativeTypes.i16Id, {"std::int_16"});

		nativeTypes.u32Id = Create();
		Add<CNativeDecl, CType>(nativeTypes.u32Id);
		Add<CIdentifier>(nativeTypes.u32Id, {"U32"});
		Add<CCppNativeName>(nativeTypes.u32Id, {"std::uint_32"});

		nativeTypes.i32Id = Create();
		Add<CNativeDecl, CType>(nativeTypes.i32Id);
		Add<CIdentifier>(nativeTypes.i32Id, {"I32"});
		Add<CCppNativeName>(nativeTypes.i32Id, {"std::int_32"});

		nativeTypes.u64Id = Create();
		Add<CNativeDecl, CType>(nativeTypes.u64Id);
		Add<CIdentifier>(nativeTypes.u64Id, {"U64"});
		Add<CCppNativeName>(nativeTypes.u64Id, {"std::uint_64"});

		nativeTypes.i64Id = Create();
		Add<CNativeDecl, CType>(nativeTypes.i64Id);
		Add<CIdentifier>(nativeTypes.i64Id, {"I64"});
		Add<CCppNativeName>(nativeTypes.i64Id, {"std::int_64"});

		nativeTypes.stringId = Create();
		Add<CNativeDecl, CType>(nativeTypes.stringId);
		Add<CIdentifier>(nativeTypes.stringId, {"String"});
		Add<CCppNativeName>(nativeTypes.stringId, {"std::string"});
	}

	void Tree::CachePools()
	{
		AssurePool<CParent>();
		AssurePool<CChild>();

		parentView = MakeOwned<TFilter<TInclude<CParent>>>(Filter<CParent>());
		childView  = MakeOwned<TFilter<TInclude<CChild>>>(Filter<CChild>());
	}

	void Tree::CopyFrom(const Tree& other)
	{
		Reset();

		// Copy entities
		idRegistry = other.idRegistry;

		// Copy component pools
		for (const PoolInstance& otherPool : other.pools)
		{
			pools.AddSorted(PoolInstance{otherPool});
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

		CachePools();
	}
}    // namespace Rift::AST
