// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"

#include <Templates/TypeList.h>


namespace Rift
{
	enum class AccessMode : u8
	{
		Read,
		Write
	};

	struct TypeAccess
	{
		Refl::TypeId typeId = Refl::TypeId::None();
		AccessMode mode     = AccessMode::Read;


		constexpr TypeAccess() = default;
		constexpr TypeAccess(Refl::TypeId typeId, AccessMode mode) : typeId{typeId}, mode{mode} {}
	};

	template<typename T, AccessMode inMode>
	struct TTypeAccess : TypeAccess
	{
		using Type = Mut<T>;

		constexpr TTypeAccess() : TypeAccess(GetTypeId<T>(), inMode) {}
	};

	template<typename T>
	struct TRead : public TTypeAccess<T, AccessMode::Read>
	{};

	template<typename T>
	struct TWrite : public TTypeAccess<T, AccessMode::Write>
	{};

	template<typename T>
	struct TTypeAccessInfo
	{
		using Type                       = Mut<T>;
		static constexpr AccessMode mode = AccessMode::Read;
	};
	template<typename T>
		requires Derived<T, TypeAccess>
	struct TTypeAccessInfo<T>
	{
		using Type                       = typename T::Type;
		static constexpr AccessMode mode = T().mode;
	};
	template<typename T>
	using AsComponent = typename TTypeAccessInfo<T>::Type;


	template<typename... T>
	struct TAccess
	{
		template<typename... K>
		friend struct TAccess;

		using Components    = TTypeList<T...>;
		using RawComponents = TTypeList<AsComponent<T>...>;

	private:
		AST::Tree& ast;
		TTuple<AST::TPool<AsComponent<T>>*...> pools;


	public:
		TAccess(AST::Tree& ast) : ast{ast}, pools{&ast.AssurePool<AsComponent<T>>()...} {}
		TAccess(const TAccess& other) : ast{other.ast}, pools{other.pools} {}

		// Construct a child access (super-set) from another access
		template<typename... T2>
		TAccess(const TAccess<T2...>& other)
		    : ast{other.ast}, pools{std::get<AST::TPool<AsComponent<T>>*>(other.pools)...}
		{
			using Other = TAccess<T2...>;
			static_assert((Other::template HasType<T>() && ...),
			    "Parent TAccess lacks one or more dependencies from a child TAccess.");

			static_assert(
			    ((Other::template IsWritable<T>() || TTypeAccessInfo<T>::mode != AccessMode::Write)
			        && ...),
			    "Parent TAccess lacks one or more *mutable* dependencies from a child TAccess.");
		}

		template<typename C>
		AST::TPool<Mut<C>>* GetPool() const requires(IsMutable<C>)
		{
			static_assert(IsWritable<C>(), "Can't modify components of this type");
			if constexpr (IsWritable<C>())    // Prevent missleading errors if condition fails
			{
				return std::get<AST::TPool<Mut<C>>*>(pools);
			}
			return nullptr;
		}

		template<typename C>
		const AST::TPool<Mut<C>>* GetPool() const requires(IsConst<C>)
		{
			static_assert(IsReadable<C>(), "Can't read components of this type");
			if constexpr (IsReadable<C>())    // Prevent missleading errors if condition fails
			{
				return std::get<AST::TPool<Mut<C>>*>(pools);
			}
			return nullptr;
		}

		template<typename C>
		AST::TPool<Mut<C>>& AssurePool() const requires(IsMutable<C>)
		{
			return *GetPool<C>();
		}

		template<typename C>
		const AST::TPool<Mut<C>>& AssurePool() const requires(IsConst<C>)
		{
			return *GetPool<C>();
		}

		bool IsValid(AST::Id id) const
		{
			return ast.IsValid(id);
		}

		template<typename... C>
		bool Has(AST::Id id) const
		{
			return (GetPool<const C>()->Has(id) && ...);
		}

		template<typename C>
		decltype(auto) Add(AST::Id id, C&& value = {}) const requires(IsSame<C, Mut<C>>)
		{
			return GetPool<C>()->Add(id, Forward<C>(value));
		}
		template<typename C>
		decltype(auto) Add(AST::Id id, const C& value) const requires(IsSame<C, Mut<C>>)
		{
			return GetPool<C>()->Add(id, value);
		}

		// Add component to many entities (if they dont have it already)
		template<typename C>
		decltype(auto) Add(TSpan<const AST::Id> ids, const C& value = {}) const
		{
			return GetPool<C>()->Add(ids.begin(), ids.end(), value);
		}

		// Add component to an entities (if they dont have it already)
		template<typename... C>
		void Add(AST::Id id) const requires((IsSame<C, Mut<C>> && ...) && sizeof...(C) > 1)
		{
			(Add<C>(id), ...);
		}

		// Add components to many entities (if they dont have it already)
		template<typename... C>
		void Add(TSpan<const AST::Id> ids) const
		    requires((IsSame<C, Mut<C>> && ...) && sizeof...(C) > 1)
		{
			(Add<C>(ids), ...);
		}


		template<typename C>
		void Remove(const AST::Id id) const requires(IsSame<C, Mut<C>>)
		{
			GetPool<C>()->Remove(id);
		}
		template<typename... C>
		void Remove(TSpan<const AST::Id> ids) const requires(IsSame<C, Mut<C>>&&...)
		{
			(GetPool<C>()->Remove(ids), ...);
		}

		template<typename C>
		C& Get(AST::Id id) const
		{
			return GetPool<C>()->Get(id);
		}

		template<typename C>
		C* TryGet(AST::Id id) const
		{
			return GetPool<C>()->TryGet(id);
		}

		template<typename C>
		C& GetOrAdd(AST::Id id) const requires(IsMutable<C>)
		{
			return GetPool<C>()->GetOrAdd(id);
		}

		i32 Size() const
		{
			static_assert(sizeof...(T) == 1, "Can only get the size of single component accesses");
			return GetPool<T...>()->Size();
		}

		AST::Tree& GetAST() const
		{
			return ast;
		}


		template<typename C>
		static constexpr bool HasType()
		{
			return ListContains<RawComponents, AsComponent<C>>();
		}

		template<typename C>
		static constexpr bool IsReadable()
		{
			return HasType<C>();
		}

		template<typename C>
		static constexpr bool IsWritable()
		{
			return IsMutable<C> && ListContains<Components, TWrite<AsComponent<C>>>();
		}
	};

	template<typename... T>
	using TAccessRef = const TAccess<T...>&;


	struct Access
	{
	protected:

		AST::Tree& ast;
		TArray<TypeAccess> types;
		TArray<AST::Pool*> pools;


	public:
		Access(AST::Tree& ast, const TArray<Refl::TypeId>& types) : ast{ast} {}

		template<typename... T>
		Access(TAccessRef<T...> access) : ast{access.ast}
		{}

		template<typename C>
		AST::TPool<Mut<C>>* GetPool() const requires(IsMutable<C>)
		{
			return nullptr;
		}

		template<typename C>
		const AST::TPool<Mut<C>>* GetPool() const requires(IsConst<C>)
		{
			return nullptr;
		}

	private :

		i32
		GetPoolIndex() const
		{
			return 0;
		}
	};
}    // namespace Rift
