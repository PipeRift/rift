// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/CModule.h"
#include "AST/Components/CNamespace.h"
#include "AST/Id.h"

#include <Pipe/Core/GenericEnums.h>
#include <Pipe/Math/Math.h>
#include <Pipe/PipeECS.h>
#include <Pipe/Reflect/EnumType.h>


namespace rift::AST
{
	Namespace GetNamespace(p::TAccessRef<CNamespace, CChild, CModule> access, Id id);
	Namespace GetParentNamespace(p::TAccessRef<CNamespace, CChild, CModule> access, Id id);

	/**
	 * Find an id from a given namespace
	 * @param access access to the needed components
	 * @param ns namespace to find the id to
	 * @param rootIds entity ids. If nullptr, roots are resolved from ecs context.
	 * @return Id found or NoId
	 */
	Id FindIdFromNamespace(p::TAccessRef<CNamespace, CChild, CParent> access, const Namespace& ns,
	    const p::TArray<Id>* rootIds = nullptr);

	p::Tag GetName(p::TAccessRef<CNamespace> access, Id id);
	p::Tag GetNameUnsafe(p::TAccessRef<CNamespace> access, Id id);
	p::String GetFullName(
	    p::TAccessRef<CNamespace, CChild, CModule> access, Id id, bool localNamespace = false);
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
