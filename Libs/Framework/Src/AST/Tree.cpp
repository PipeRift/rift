// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Components/CIdentifier.h"
#include "AST/Components/CNativeDecl.h"
#include "AST/Components/Tags/CType.h"
#include "AST/Tree.h"
#include "Compiler/Cpp/Components/CCppNativeName.h"


namespace Rift::AST
{
	Tree::Tree() : registry{}, parentView(MakeView<CParent>()), childrenView(MakeView<CChildren>())
	{
		SetupNativeTypes();
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
		AddComponent<CNativeDecl>(boolType);
		AddComponent<CIdentifier>(boolType, "bool");
		AddComponent<CType>(boolType);
		AddComponent<CCppNativeName>(boolType, "bool");

		AST::Id floatType = Create();
		AddComponent<CNativeDecl>(floatType);
		AddComponent<CIdentifier>(floatType, "float");
		AddComponent<CType>(floatType);
		AddComponent<CCppNativeName>(floatType, "float");

		AST::Id doubleType = Create();
		AddComponent<CNativeDecl>(doubleType);
		AddComponent<CIdentifier>(doubleType, "double");
		AddComponent<CType>(doubleType);
		AddComponent<CCppNativeName>(doubleType, "double");

		AST::Id u8Type = Create();
		AddComponent<CNativeDecl>(u8Type);
		AddComponent<CIdentifier>(u8Type, "u8");
		AddComponent<CType>(u8Type);
		AddComponent<CCppNativeName>(u8Type, "std::uint_8");

		AST::Id i8Type = Create();
		AddComponent<CNativeDecl>(i8Type);
		AddComponent<CIdentifier>(i8Type, "i8");
		AddComponent<CType>(i8Type);
		AddComponent<CCppNativeName>(i8Type, "std::int_8");

		AST::Id u16Type = Create();
		AddComponent<CNativeDecl>(u16Type);
		AddComponent<CIdentifier>(u16Type, "u16");
		AddComponent<CType>(u16Type);
		AddComponent<CCppNativeName>(u16Type, "std::uint_16");

		AST::Id i16Type = Create();
		AddComponent<CNativeDecl>(i16Type);
		AddComponent<CIdentifier>(i16Type, "i16");
		AddComponent<CType>(i16Type);
		AddComponent<CCppNativeName>(i16Type, "std::int_16");

		AST::Id u32Type = Create();
		AddComponent<CNativeDecl>(u32Type);
		AddComponent<CIdentifier>(u32Type, "u32");
		AddComponent<CType>(u32Type);
		AddComponent<CCppNativeName>(u32Type, "std::uint_32");

		AST::Id i32Type = Create();
		AddComponent<CNativeDecl>(i32Type);
		AddComponent<CIdentifier>(i32Type, "i32");
		AddComponent<CType>(i32Type);
		AddComponent<CCppNativeName>(i32Type, "std::int_32");

		AST::Id u64Type = Create();
		AddComponent<CNativeDecl>(u64Type);
		AddComponent<CIdentifier>(u64Type, "u64");
		AddComponent<CType>(u64Type);
		AddComponent<CCppNativeName>(u64Type, "std::uint_64");

		AST::Id i64Type = Create();
		AddComponent<CNativeDecl>(i64Type);
		AddComponent<CIdentifier>(i64Type, "i64");
		AddComponent<CType>(i64Type);
		AddComponent<CCppNativeName>(i64Type, "std::int_64");
	}
}    // namespace Rift::AST
