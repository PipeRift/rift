// Copyright 2015-2023 Piperift - All rights reserved

#include "AST/Tree.h"

#include "AST/Components/CNamespace.h"
#include "AST/Components/Declarations.h"
#include "AST/Statics/SModules.h"
#include "AST/Statics/STypes.h"

#include <PipeECS.h>


namespace rift::ast
{
	p::TBroadcast<Tree&> Tree::onInit{};


	Tree::Tree()
	{
		SetupNativeTypes();
		onInit(*this);
	}

	Tree::Tree(const Tree& other) noexcept : p::EntityContext(other)
	{
		CopyFrom(other);
	}
	Tree::Tree(Tree&& other) noexcept : p::EntityContext(p::Move(other))
	{
		MoveFrom(p::Move(other));
	}
	Tree& Tree::operator=(const Tree& other) noexcept
	{
		p::EntityContext::operator=(other);
		CopyFrom(other);
		return *this;
	}
	Tree& Tree::operator=(Tree&& other) noexcept
	{
		EntityContext::operator=(p::Move(other));
		MoveFrom(p::Move(other));
		return *this;
	}

	const p::TBroadcast<Tree&>& Tree::OnInit()
	{
		return onInit;
	}

	void Tree::SetupNativeTypes()
	{
		// Remove any previous native types
		Destroy(p::FindAllIdsWith<CDeclNative>(*this));

		nativeTypes.boolId = Create();
		Add<CDeclType, CDeclNative>(nativeTypes.boolId);
		Add(nativeTypes.boolId, CNamespace{"Bool"});

		nativeTypes.floatId = Create();
		Add<CDeclType, CDeclNative>(nativeTypes.floatId);
		Add(nativeTypes.floatId, CNamespace{"Float"});

		nativeTypes.doubleId = Create();
		Add<CDeclType, CDeclNative>(nativeTypes.doubleId);
		Add(nativeTypes.doubleId, CNamespace{"Double"});

		nativeTypes.u8Id = Create();
		Add<CDeclType, CDeclNative>(nativeTypes.u8Id);
		Add(nativeTypes.u8Id, CNamespace{"U8"});

		nativeTypes.i8Id = Create();
		Add<CDeclType, CDeclNative>(nativeTypes.i8Id);
		Add(nativeTypes.i8Id, CNamespace{"I8"});

		nativeTypes.u16Id = Create();
		Add<CDeclType, CDeclNative>(nativeTypes.u16Id);
		Add(nativeTypes.u16Id, CNamespace{"U16"});

		nativeTypes.i16Id = Create();
		Add<CDeclType, CDeclNative>(nativeTypes.i16Id);
		Add(nativeTypes.i16Id, CNamespace{"I16"});

		nativeTypes.u32Id = Create();
		Add<CDeclType, CDeclNative>(nativeTypes.u32Id);
		Add(nativeTypes.u32Id, CNamespace{"U32"});

		nativeTypes.i32Id = Create();
		Add<CDeclType, CDeclNative>(nativeTypes.i32Id);
		Add(nativeTypes.i32Id, CNamespace{"I32"});

		nativeTypes.u64Id = Create();
		Add<CDeclType, CDeclNative>(nativeTypes.u64Id);
		Add(nativeTypes.u64Id, CNamespace{"U64"});

		nativeTypes.i64Id = Create();
		Add<CDeclType, CDeclNative>(nativeTypes.i64Id);
		Add(nativeTypes.i64Id, CNamespace{"I64"});

		nativeTypes.stringId = Create();
		Add<CDeclType, CDeclNative>(nativeTypes.stringId);
		Add(nativeTypes.stringId, CNamespace{"String"});
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

	p::String Tree::DumpPools()
	{
		p::String text;

		text.append("Pools: \n");
		for (const auto& pool : GetPools())
		{
			p::Type* type = p::TypeRegistry::Get().FindType(pool.GetId());
			p::Strings::FormatTo(text, "- {} x{}\n",
			    type ? type->GetName() : p::StringView{"NotReflected"}, pool.GetPool()->Size());
		}

		return text;
	}
}    // namespace rift::ast
