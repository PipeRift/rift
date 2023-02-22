// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/CModule.h"
#include "AST/Components/CNamespace.h"
#include "AST/Id.h"

#include <Pipe/Core/GenericEnums.h>
#include <Pipe/ECS/Access.h>
#include <Pipe/ECS/Components/CChild.h>
#include <Pipe/ECS/Utils/Hierarchy.h>
#include <Pipe/Math/Math.h>
#include <Pipe/Reflect/EnumType.h>


namespace rift::AST
{
	using LocalNamespace = p::YesNo;

	struct Namespace : public Struct
	{
		STRUCT(Namespace, Struct)

		static constexpr i32 scopeCount = 8;
		Tag scopes[scopeCount];    // TODO: Implement Inline arrays


		Namespace() = default;
		template<i32 M>
		Namespace(Tag scopes[M]) requires(M <= scopeCount) : scopes{scopes}
		{}
		Namespace(StringView value);
		// Prevent initializer list from stealing string constructor
		Namespace(const String& value) : Namespace(StringView{value}) {}
		Namespace(std::initializer_list<Tag> values)
		{
			const i32 size = p::math::Min(i32(values.size()), scopeCount);
			for (i32 i = 0; i < size; ++i)
			{
				scopes[i] = *(values.begin() + i);
			}
		}

		bool Equals(const Namespace& other) const;
		bool IsEmpty() const;
		i32 Size() const;
		bool Contains(const Namespace& other) const;
		p::String ToString(LocalNamespace isLocal = LocalNamespace::No) const;
		Tag& First()
		{
			return scopes[0];
		}
		Tag First() const
		{
			return scopes[0];
		}
		Tag& Last()
		{
			const i32 size      = Size();
			const i32 lastIndex = size > 0 ? (size - 1) : 0;    // Is Size is 0, last is first
			return scopes[lastIndex];
		}
		const Tag& Last() const
		{
			const i32 size      = Size();
			const i32 lastIndex = size > 0 ? (size - 1) : 0;    // Is Size is 0, last is first
			return scopes[lastIndex];
		}
		bool operator==(const Namespace& other) const
		{
			return Equals(other);
		}
		Tag operator[](i32 index) const
		{
			Check(index >= 0 && index < scopeCount);
			return scopes[index];
		}
		operator bool() const
		{
			return !IsEmpty();
		}

		Tag* begin()
		{
			return scopes;
		}
		const Tag* begin() const
		{
			return scopes;
		}
		Tag* end()
		{
			return scopes + Size();
		}
		const Tag* end() const
		{
			return scopes + Size();
		}

		void Read(p::Reader& ct);
		void Write(p::Writer& ct) const;
	};


	Namespace GetNamespace(TAccessRef<CNamespace, CChild, CModule> access, Id id);
	Namespace GetParentNamespace(TAccessRef<CNamespace, CChild, CModule> access, Id id);

	/**
	 * Find an id from a given namespace
	 * @param access access to the needed components
	 * @param ns namespace to find the id to
	 * @param rootIds entity ids. If nullptr, roots are resolved from ecs context.
	 * @return Id found or NoId
	 */
	Id FindIdFromNamespace(TAccessRef<CNamespace, CChild, CParent> access, const Namespace& ns,
	    const TArray<Id>* rootIds = nullptr);

	Tag GetName(TAccessRef<CNamespace> access, Id id);
	Tag GetNameUnsafe(TAccessRef<CNamespace> access, Id id);
	p::String GetFullName(TAccessRef<CNamespace, CChild, CModule> access, Id id,
	    LocalNamespace localNamespace = LocalNamespace::No);
}    // namespace rift::AST


namespace p
{
	template<>
	struct TFlags<rift::AST::Namespace> : public DefaultTFlags
	{
		enum
		{
			HasMemberSerialize = true
		};
	};
}    // namespace p
