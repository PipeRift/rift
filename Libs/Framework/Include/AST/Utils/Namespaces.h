// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CChild.h"
#include "AST/Components/CModule.h"
#include "AST/Components/CNamespace.h"
#include "AST/Id.h"
#include "AST/Utils/Hierarchy.h"
#include "Pipe/Math/Math.h"

#include <Pipe/Core/GenericEnums.h>
#include <Pipe/ECS/Access.h>
#include <Pipe/Reflect/EnumType.h>


namespace rift::AST
{
	using LocalNamespace = p::YesNo;

	struct Namespace
	{
		static constexpr i32 scopeCount = 8;
		Name scopes[scopeCount];    // TODO: Implement Inline arrays


		Namespace() = default;
		template<i32 M>
		Namespace(Name scopes[M]) requires(M <= scopeCount) : scopes{scopes}
		{}
		Namespace(std::initializer_list<Name> values)
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
		Name& GetFirstScope()
		{
			return scopes[0];
		}
		Name GetFirstScope() const
		{
			return scopes[0];
		}
		Name& GetLastScope()
		{
			return scopes[scopeCount - 1];
		}
		Name GetLastScope() const
		{
			return scopes[scopeCount - 1];
		}
		bool operator==(const Namespace& other) const
		{
			return Equals(other);
		}
		operator bool() const
		{
			return IsEmpty();
		}

		Name* begin()
		{
			return scopes;
		}
		const Name* begin() const
		{
			return scopes;
		}
		Name* end()
		{
			return scopes + Size();
		}
		const Name* end() const
		{
			return scopes + Size();
		}
	};


	Namespace GetNamespace(TAccessRef<CNamespace, CChild, CModule> access, Id id);

	Name GetName(TAccessRef<CNamespace> access, Id id);
	Name GetNameChecked(TAccessRef<CNamespace> access, Id id);
	p::String GetFullName(TAccessRef<CNamespace, CChild, CModule> access, Id id,
	    LocalNamespace localNamespace = LocalNamespace::No);
	p::String GetFullNameChecked(TAccessRef<CNamespace, CChild, CModule> access, Id id,
	    LocalNamespace localNamespace = LocalNamespace::No);
}    // namespace rift::AST
