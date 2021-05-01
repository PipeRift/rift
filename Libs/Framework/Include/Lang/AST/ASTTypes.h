// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <entt/entt.hpp>

namespace Rift::AST
{
	using Id          = entt::entity;
	using VersionType = entt::entt_traits<Id>::version_type;

	template <typename... Type>
	using TExclude = entt::exclude_t<Type...>;
}    // namespace Rift::AST
