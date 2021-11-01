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
	Tree::Tree() : registry{}
	{
		SetupNativeTypes();
		CachePools();
	}

	Tree::Tree(Tree&& other) noexcept
	{
		registry = Move(other.registry);
		CachePools();
	}
	Tree& Tree::operator=(Tree&& other) noexcept
	{
		registry = Move(other.registry);
		CachePools();
		return *this;
	}

	Id Tree::Create()
	{
		return registry.create();
	}
	Id Tree::Create(const Id hint)
	{
		return registry.create(Move(hint));
	}

	void Tree::Destroy(const Id node)
	{
		registry.destroy(node);
	}

	void Tree::Destroy(const Id node, const VersionType version)
	{
		registry.destroy(node, version);
	}

	void Tree::SetupNativeTypes()
	{
		nativeTypes.boolId = Create();
		Add<CNativeDecl, CType>(nativeTypes.boolId);
		Add<CIdentifier>(nativeTypes.boolId, "bool");
		Add<CCppNativeName>(nativeTypes.boolId, "bool");

		nativeTypes.floatId = Create();
		Add<CNativeDecl, CType>(nativeTypes.floatId);
		Add<CIdentifier>(nativeTypes.floatId, "float");
		Add<CCppNativeName>(nativeTypes.floatId, "float");

		nativeTypes.doubleId = Create();
		Add<CNativeDecl, CType>(nativeTypes.doubleId);
		Add<CIdentifier>(nativeTypes.doubleId, "double");
		Add<CCppNativeName>(nativeTypes.doubleId, "double");

		nativeTypes.u8Id = Create();
		Add<CNativeDecl, CType>(nativeTypes.u8Id);
		Add<CIdentifier>(nativeTypes.u8Id, "u8");
		Add<CCppNativeName>(nativeTypes.u8Id, "std::uint_8");

		nativeTypes.i8Id = Create();
		Add<CNativeDecl, CType>(nativeTypes.i8Id);
		Add<CIdentifier>(nativeTypes.i8Id, "i8");
		Add<CCppNativeName>(nativeTypes.i8Id, "std::int_8");

		nativeTypes.u16Id = Create();
		Add<CNativeDecl, CType>(nativeTypes.u16Id);
		Add<CIdentifier>(nativeTypes.u16Id, "u16");
		Add<CCppNativeName>(nativeTypes.u16Id, "std::uint_16");

		nativeTypes.i16Id = Create();
		Add<CNativeDecl, CType>(nativeTypes.i16Id);
		Add<CIdentifier>(nativeTypes.i16Id, "i16");
		Add<CCppNativeName>(nativeTypes.i16Id, "std::int_16");

		nativeTypes.u32Id = Create();
		Add<CNativeDecl, CType>(nativeTypes.u32Id);
		Add<CIdentifier>(nativeTypes.u32Id, "u32");
		Add<CCppNativeName>(nativeTypes.u32Id, "std::uint_32");

		nativeTypes.i32Id = Create();
		Add<CNativeDecl, CType>(nativeTypes.i32Id);
		Add<CIdentifier>(nativeTypes.i32Id, "i32");
		Add<CCppNativeName>(nativeTypes.i32Id, "std::int_32");

		nativeTypes.u64Id = Create();
		Add<CNativeDecl, CType>(nativeTypes.u64Id);
		Add<CIdentifier>(nativeTypes.u64Id, "u64");
		Add<CCppNativeName>(nativeTypes.u64Id, "std::uint_64");

		nativeTypes.i64Id = Create();
		Add<CNativeDecl, CType>(nativeTypes.i64Id);
		Add<CIdentifier>(nativeTypes.i64Id, "i64");
		Add<CCppNativeName>(nativeTypes.i64Id, "std::int_64");
	}

	void Tree::CachePools()
	{
		registry.prepare<CChild>();
		registry.prepare<CParent>();

		childView  = MakeOwned<View<TExclude<>, CChild>>(MakeView<CChild>());
		parentView = MakeOwned<View<TExclude<>, CParent>>(MakeView<CParent>());
	}

	void Tree::CopyFrom(const Tree& other)
	{
		Reset();

		// Copy entities
		registry.assign(other.registry.data(), other.registry.data() + other.registry.size(),
		    other.registry.destroyed());

		// Copy components
		other.registry.visit([&other, this](const auto info) {
			other.registry.storage(info)->copy_to(registry);
		});
		CachePools();

		// Copy non-transient unique components
		// TODO: Use reflection for this
		SetUnique<CModulesUnique>(other.GetUnique<CModulesUnique>());
		SetUnique<CTypesUnique>(other.GetUnique<CTypesUnique>());
	}
}    // namespace Rift::AST
