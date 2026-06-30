// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "AST/Components/CModule.h"
#include "AST/Components/CNamespace.h"
#include "AST/Id.h"

#include <Pipe/Core/GenericEnums.h>
#include <PipeECS.h>
#include <PipeMath.h>


namespace rift::ast
{
	Namespace GetNamespace(p::TIdScopeRef<CNamespace, CChild, CModule> scope, Id id);
	Namespace GetParentNamespace(p::TIdScopeRef<CNamespace, CChild, CModule> scope, Id id);

	/**
	 * Find an id from a given namespace
	 * @param scope scope to the needed components
	 * @param ns namespace to find the id to
	 * @param rootIds entity ids. If nullptr, roots are resolved from ecs context.
	 * @return Id found or NoId
	 */
	Id FindIdFromNamespace(p::TIdScopeRef<CNamespace, CChild, CParent> scope, const Namespace& ns,
	    const p::TArray<Id>* rootIds = nullptr);

	p::Tag GetName(p::TIdScopeRef<CNamespace> scope, Id id);
	p::Tag GetNameUnsafe(p::TIdScopeRef<CNamespace> scope, Id id);
	p::String GetFullName(p::TIdScopeRef<CNamespace, CChild, CModule> scope, Id id,
	    bool localNamespace = false, char separator = '.');
}    // namespace rift::ast


namespace p
{
	template<>
	struct TFlags<rift::ast::Namespace> : public DefaultTFlags
	{
		enum
		{
			HasMemberSerialize = true
		};
	};
}    // namespace p
