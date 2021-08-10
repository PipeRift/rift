// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include <Platform/Platform.h>

#include <entt/entity/poly_storage.hpp>
#include <entt/entity/registry.hpp>


template <typename... Type>
entt::type_list<Type...> as_type_list(const entt::type_list<Type...>&);

// clang-format off
template <typename Entity>
struct PolyStorage : entt::type_list_cat_t<
    decltype(as_type_list(std::declval<entt::Storage<Entity>>())),
        entt::type_list<
            const void*(const Entity) const,
            void(entt::basic_registry<Entity>&) const,
            void(entt::basic_registry<Entity>&, const Entity* entity, const void* components, Rift::sizet size)
        >
    >
{
	using entity_type = Entity;
	using size_type   = std::size_t;

	template <typename Base>
	struct type : entt::Storage<Entity>::template type<Base>
	{
		static constexpr auto base =
		    decltype(as_type_list(std::declval<entt::Storage<Entity>>()))::size;

		void* get(const entity_type entity)
		{
			const auto* self = this;
			return const_cast<void*>(self->get(entity));
		}

		const void* get(const entity_type entity) const
		{
			return entt::poly_call<base + 0>(*this, entity);
		}

		void copy_to(entt::basic_registry<Entity>& other) const
		{
			entt::poly_call<base + 1>(*this, other);
		}

		void insert(entt::basic_registry<Entity> &owner, const Entity* entities, const void* components, Rift::sizet size)
		{
			entt::poly_call<base + 2>(*this, owner, entities, components, size);
		}
	};

	template <typename Type>
	struct members
	{
		static const void* get(const Type& self, const entity_type entity)
		{
			if constexpr(!std::is_empty_v<typename Type::value_type>)
			{
				return static_cast<const void*>(&self.get(entity));
			}
			return nullptr;
		}

		static void copy_to(const Type& self, entt::basic_registry<entity_type>& other)
		{
			other.prepare<typename Type::value_type>();
			auto& otherPool = other.storage(entt::type_id<typename Type::value_type>());

			if constexpr(!std::is_empty_v<typename Type::value_type>)
			{
				if (self.raw())
				{
					otherPool->insert(other, self.data(), *self.raw(), self.size());
				}
			}
			else
			{
				otherPool->insert(other, self.data(), nullptr, self.size());
			}
		}

		static void insert(Type& self, entt::basic_registry<entity_type> &owner, const Entity* entities, const void* components, Rift::sizet size)
		{
			if constexpr(!std::is_empty_v<typename Type::value_type>)
			{
				self.insert(owner, entities, entities + size, reinterpret_cast<const typename Type::value_type*>(components));
			}
			else
			{
				self.insert(owner, entities, entities + size);
			}
		}
	};

	template <typename Type>
	using impl = entt::value_list_cat_t<
        typename entt::Storage<Entity>::template impl<Type>,
            entt::value_list<
            &members<Type>::get,
            &members<Type>::copy_to,
			&members<Type>::insert
        >
    >;
};
// clang-format on
