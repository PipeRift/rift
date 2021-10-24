// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"

#include <entt/entity/observer.hpp>


namespace Rift::AST
{
	template<typename... Types>
	using Matcher = entt::matcher<Types...>;

	template<typename...>
	struct Collector;

	template<>
	struct Collector<>
	{
		template<typename... AllOf, typename... NoneOf>
		static constexpr auto Group(TExclude<NoneOf...> = {}) ENTT_NOEXCEPT
		{
			return Collector<Matcher<TTypeList<>, TTypeList<>, TTypeList<NoneOf...>, AllOf...>>{};
		}

		template<typename AnyOf>
		static constexpr auto Update() ENTT_NOEXCEPT
		{
			return Collector<Matcher<TTypeList<>, TTypeList<>, AnyOf>>{};
		}
	};

	template<typename... Reject, typename... Require, typename... Rule, typename... Other>
	struct Collector<Matcher<TTypeList<Reject...>, TTypeList<Require...>, Rule...>, Other...>
	{
		using current_type = Matcher<TTypeList<Reject...>, TTypeList<Require...>, Rule...>;

		template<typename... AllOf, typename... NoneOf>
		static constexpr auto Group(TExclude<NoneOf...> = {}) ENTT_NOEXCEPT
		{
			return Collector<Matcher<TTypeList<>, TTypeList<>, TTypeList<NoneOf...>, AllOf...>,
			    current_type, Other...>{};
		}

		template<typename AnyOf>
		static constexpr auto Update() ENTT_NOEXCEPT
		{
			return Collector<Matcher<TTypeList<>, TTypeList<>, AnyOf>, current_type, Other...>{};
		}

		template<typename... AllOf, typename... NoneOf>
		static constexpr auto Where(TExclude<NoneOf...> = {}) ENTT_NOEXCEPT
		{
			using extended_type =
			    Matcher<TTypeList<Reject..., NoneOf...>, TTypeList<Require..., AllOf...>, Rule...>;
			return Collector<extended_type, Other...>{};
		}
	};


	/*! @brief Variable template used to ease the definition of collectors. */
	inline constexpr Collector<> collector{};


	struct Observer
	{
		using EnTTObserver = entt::basic_observer<Id>;
		using Iterator     = typename EnTTObserver::iterator;

	private:
		EnTTObserver observer;


	public:
		Observer() : observer{} {}

		Observer(const Observer&) = delete;
		Observer(Observer&&)      = delete;

		template<typename... Matcher>
		Observer(Tree& ast, Collector<Matcher...>)
		    : observer{ast.GetRegistry(), entt::basic_collector<Matcher...>{}}
		{}

		Observer& operator=(const Observer&) = delete;
		Observer& operator=(Observer&&) = delete;

		template<typename... Matcher>
		void Connect(Tree& ast, Collector<Matcher...>)
		{
			observer.connect<Matcher...>(ast.GetRegistry(), entt::basic_collector<Matcher...>{});
		}

		void Disconnect()
		{
			observer.disconnect();
		}

		sizet Size() const
		{
			return observer.size();
		}

		bool IsEmpty() const
		{
			return observer.empty();
		}

		const Id* Data() const
		{
			return observer.data();
		}

		Iterator begin() const
		{
			return observer.begin();
		}

		Iterator end() const
		{
			return observer.end();
		}

		void Clear()
		{
			observer.clear();
		}

		template<typename Func>
		void Each(Func func) const
		{
			observer.each(std::move(func));
		}

		template<typename Func>
		void Each(Func func)
		{
			observer.each(std::move(func));
		}
	};
}    // namespace Rift::AST
