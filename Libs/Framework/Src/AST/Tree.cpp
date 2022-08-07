// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Tree.h"

#include "AST/Components/CDeclNative.h"
#include "AST/Components/CNamespace.h"
#include "AST/Components/CType.h"
#include "AST/Statics/SModules.h"
#include "AST/Statics/STypes.h"

#include <Pipe/ECS/Filtering.h>


namespace rift::AST
{
	Tree::Tree()
	{
		SetupNativeTypes();
	}

	Tree::Tree(const Tree& other) noexcept : ecs::Context(other)
	{
		CopyFrom(other);
	}
	Tree::Tree(Tree&& other) noexcept : ecs::Context(Move(other))
	{
		MoveFrom(Move(other));
	}
	Tree& Tree::operator=(const Tree& other) noexcept
	{
		ecs::Context::operator=(other);
		CopyFrom(other);
		return *this;
	}
	Tree& Tree::operator=(Tree&& other) noexcept
	{
		ecs::Context::operator=(Move(other));
		MoveFrom(Move(other));
		return *this;
	}

	void Tree::SetupNativeTypes()
	{
		// Remove any previous native types
		Destroy(ListAll<CDeclNative>(*this));

		nativeTypes.boolId = Create();
		Add<CType, CDeclNative>(nativeTypes.boolId);
		Add<CNamespace>(nativeTypes.boolId, "Bool");

		nativeTypes.floatId = Create();
		Add<CType, CDeclNative>(nativeTypes.floatId);
		Add<CNamespace>(nativeTypes.floatId, "Float");

		nativeTypes.doubleId = Create();
		Add<CType, CDeclNative>(nativeTypes.doubleId);
		Add<CNamespace>(nativeTypes.doubleId, "Double");

		nativeTypes.u8Id = Create();
		Add<CType, CDeclNative>(nativeTypes.u8Id);
		Add<CNamespace>(nativeTypes.u8Id, "U8");

		nativeTypes.i8Id = Create();
		Add<CType, CDeclNative>(nativeTypes.i8Id);
		Add<CNamespace>(nativeTypes.i8Id, "I8");

		nativeTypes.u16Id = Create();
		Add<CType, CDeclNative>(nativeTypes.u16Id);
		Add<CNamespace>(nativeTypes.u16Id, "U16");

		nativeTypes.i16Id = Create();
		Add<CType, CDeclNative>(nativeTypes.i16Id);
		Add<CNamespace>(nativeTypes.i16Id, "I16");

		nativeTypes.u32Id = Create();
		Add<CType, CDeclNative>(nativeTypes.u32Id);
		Add<CNamespace>(nativeTypes.u32Id, "U32");

		nativeTypes.i32Id = Create();
		Add<CType, CDeclNative>(nativeTypes.i32Id);
		Add<CNamespace>(nativeTypes.i32Id, "I32");

		nativeTypes.u64Id = Create();
		Add<CType, CDeclNative>(nativeTypes.u64Id);
		Add<CNamespace>(nativeTypes.u64Id, "U64");

		nativeTypes.i64Id = Create();
		Add<CType, CDeclNative>(nativeTypes.i64Id);
		Add<CNamespace>(nativeTypes.i64Id, "I64");

		nativeTypes.stringId = Create();
		Add<CType, CDeclNative>(nativeTypes.stringId);
		Add<CNamespace>(nativeTypes.stringId, "String");
	}

	void Tree::CopyFrom(const Tree& other)
	{
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
	}

	void Tree::MoveFrom(Tree&& other)
	{
		nativeTypes = other.nativeTypes;
	}
}    // namespace rift::AST
