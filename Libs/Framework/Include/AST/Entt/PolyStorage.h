// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

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
            void(entt::basic_registry<Entity>&) const
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
			return const_cast<void*>(entt::poly_call<base + 0>(*this, entity));
		}

		const void* get(const entity_type entity) const
		{
			return entt::poly_call<base + 0>(*this, entity);
		}

		void copy_to(entt::basic_registry<Entity>& other) const
		{
			entt::poly_call<base + 1>(*this, other);
		}
	};

	template <typename Type>
	struct members
	{
		static const typename Type::value_type* get(const Type& self, const entity_type entity)
		{
			return nullptr;    //&self.get(entity);
		}

		static void copy_to(const Type& self, entt::basic_registry<entity_type>& other)
		{
			const entt::basic_sparse_set<entity_type>& base = self;
			// other.template insert<typename Type::value_type>(
			//    base.rbegin(), base.rend(), self.rbegin());
		}
	};

	template <typename Type>
	using impl = entt::value_list_cat_t<
        typename entt::Storage<Entity>::template impl<Type>,
            entt::value_list<
            &members<Type>::get,
            &members<Type>::copy_to
        >
    >;
};
// clang-format on
