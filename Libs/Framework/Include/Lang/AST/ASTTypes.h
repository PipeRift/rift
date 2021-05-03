// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <Platform/Platform.h>

#include <entt/entt.hpp>


namespace Rift::AST
{
	enum class Id : u32
	{
	};

	constexpr entt::null_t NoId = entt::null;
}    // namespace Rift::AST


namespace entt
{
	template <>
	struct entt_traits<Rift::AST::Id> : public entt_traits<Rift::u32>
	{};
}    // namespace entt


namespace Rift::AST
{
	using VersionType = entt::entt_traits<Id>::version_type;

	template <typename... Type>
	using TExclude = entt::exclude_t<Type...>;
}    // namespace Rift::AST
