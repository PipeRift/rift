// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Tree.h"

#include "AST/Components/CDeclNative.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CType.h"
#include "AST/Statics/SModules.h"
#include "AST/Statics/STypes.h"

#include <ECS/Filtering.h>


namespace rift::AST
{
	Tree::Tree()
	{
		SetupNativeTypes();
	}

	Tree::Tree(const Tree& other) noexcept : ECS::Context(other)
	{
		CopyFrom(other);
	}
	Tree::Tree(Tree&& other) noexcept : ECS::Context(Move(other))
	{
		MoveFrom(Move(other));
	}
	Tree& Tree::operator=(const Tree& other) noexcept
	{
		ECS::Context::operator=(other);
		CopyFrom(other);
		return *this;
	}
	Tree& Tree::operator=(Tree&& other) noexcept
	{
		ECS::Context::operator=(Move(other));
		MoveFrom(Move(other));
		return *this;
	}

	void Tree::SetupNativeTypes()
	{
		// Remove any previous native types
		Destroy(ListAll<CDeclNative>(*this));

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
