// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <ECS.h>
#include <Strings/Name.h>


namespace Rift
{
	struct AST
	{
		using Id = ECS::EntityId;

	private:
		ECS::Registry registry;


	public:
		Id CreateClass(Name name);
		Id CreateStruct(Name name);

		template <typename Component, typename... Args>
		auto AddComponent(Id node, Args&&... args)
		{
			return registry.Emplace<Component>(node, Forward<Args>(args)...);
		}

		ECS::Registry& GetRegistry()
		{
			return registry;
		}

		const ECS::Registry& GetRegistry() const
		{
			return registry;
		}
	};
}    // namespace Rift
