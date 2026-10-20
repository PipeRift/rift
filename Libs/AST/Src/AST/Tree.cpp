// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "AST/Tree.h"

#include "AST/Components/CNamespace.h"
#include "AST/Components/Declarations.h"
#include "AST/Statics/SModules.h"
#include "AST/Statics/STypes.h"
#include "AST/Systems/CastSystem.h"

#include <PipeECS.h>


namespace rift::ast
{
	p::TBroadcast<Tree&> Tree::onInit{};


	Tree::Tree()
	{
		SetupNativeTypes();
		onInit(*this);
	}

	Tree::Tree(const Tree& other) noexcept : p::IdContext(other)
	{
		CopyFrom(other);
	}
	Tree::Tree(Tree&& other) noexcept : p::IdContext(p::Move(other))
	{
		MoveFrom(p::Move(other));
	}
	Tree& Tree::operator=(const Tree& other) noexcept
	{
		p::IdContext::operator=(other);
		CopyFrom(other);
		return *this;
	}
	Tree& Tree::operator=(Tree&& other) noexcept
	{
		IdContext::operator=(p::Move(other));
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
		RmId(*this, p::FindAllIdsWith<CDeclNative>(*this));

		auto addNativeType = [this](Id& id, const char* name)
		{
			id = AddId(*this);
			Add<CDeclType, CDeclNative>(id);
			Add(id, CNamespace{name});
		};

		addNativeType(nativeTypes.boolId, "Bool");
		addNativeType(nativeTypes.floatId, "Float");
		addNativeType(nativeTypes.doubleId, "Double");
		addNativeType(nativeTypes.u8Id, "U8");
		addNativeType(nativeTypes.i8Id, "I8");
		addNativeType(nativeTypes.u16Id, "U16");
		addNativeType(nativeTypes.i16Id, "I16");
		addNativeType(nativeTypes.u32Id, "U32");
		addNativeType(nativeTypes.i32Id, "I32");
		addNativeType(nativeTypes.u64Id, "U64");
		addNativeType(nativeTypes.i64Id, "I64");
		addNativeType(nativeTypes.stringId, "String");

		// Casts between the built-in types depend on their ids
		CastSystem::RegisterNumericCasts(*this);
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
		if (auto* castsStatic = other.TryGetStatic<SCasts>())
		{
			SetStatic<SCasts>(*castsStatic);
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
			p::TypeId type = pool.GetId();
			p::FormatTo(text, "- {} x{}\n",
			    type.IsValid() ? GetTypeName(type) : p::StringView{"NotReflected"},
			    pool.GetPool()->Size());
		}

		return text;
	}
}    // namespace rift::ast
