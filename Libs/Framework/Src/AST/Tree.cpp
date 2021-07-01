// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Components/CChild.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CNativeDecl.h"
#include "AST/Components/CParent.h"
#include "AST/Components/Tags/CType.h"
#include "AST/Tree.h"
#include "Compiler/Cpp/Components/CCppNativeName.h"


namespace Rift::AST
{
	Tree::Tree() : registry{}
	{
		SetupNativeTypes();
		CachePools();
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
		AST::Id boolType = Create();
		Add<CNativeDecl, CType>(boolType);
		Add<CIdentifier>(boolType, "bool");
		Add<CCppNativeName>(boolType, "bool");

		AST::Id floatType = Create();
		Add<CNativeDecl, CType>(floatType);
		Add<CIdentifier>(floatType, "float");
		Add<CCppNativeName>(floatType, "float");

		AST::Id doubleType = Create();
		Add<CNativeDecl, CType>(doubleType);
		Add<CIdentifier>(doubleType, "double");
		Add<CCppNativeName>(doubleType, "double");

		AST::Id u8Type = Create();
		Add<CNativeDecl, CType>(u8Type);
		Add<CIdentifier>(u8Type, "u8");
		Add<CCppNativeName>(u8Type, "std::uint_8");

		AST::Id i8Type = Create();
		Add<CNativeDecl, CType>(i8Type);
		Add<CIdentifier>(i8Type, "i8");
		Add<CCppNativeName>(i8Type, "std::int_8");

		AST::Id u16Type = Create();
		Add<CNativeDecl, CType>(u16Type);
		Add<CIdentifier>(u16Type, "u16");
		Add<CCppNativeName>(u16Type, "std::uint_16");

		AST::Id i16Type = Create();
		Add<CNativeDecl, CType>(i16Type);
		Add<CIdentifier>(i16Type, "i16");
		Add<CCppNativeName>(i16Type, "std::int_16");

		AST::Id u32Type = Create();
		Add<CNativeDecl, CType>(u32Type);
		Add<CIdentifier>(u32Type, "u32");
		Add<CCppNativeName>(u32Type, "std::uint_32");

		AST::Id i32Type = Create();
		Add<CNativeDecl, CType>(i32Type);
		Add<CIdentifier>(i32Type, "i32");
		Add<CCppNativeName>(i32Type, "std::int_32");

		AST::Id u64Type = Create();
		Add<CNativeDecl, CType>(u64Type);
		Add<CIdentifier>(u64Type, "u64");
		Add<CCppNativeName>(u64Type, "std::uint_64");

		AST::Id i64Type = Create();
		Add<CNativeDecl, CType>(i64Type);
		Add<CIdentifier>(i64Type, "i64");
		Add<CCppNativeName>(i64Type, "std::int_64");
	}

	void Tree::CachePools()
	{
		registry.prepare<CChild>();
		childPool = &registry.storage(entt::type_id<CChild>());

		registry.prepare<CParent>();
		parentPool = &registry.storage(entt::type_id<CParent>());
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
	}
}    // namespace Rift::AST
