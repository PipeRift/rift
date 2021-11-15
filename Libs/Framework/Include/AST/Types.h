// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <Platform/Platform.h>
#include <Reflection/Registry/NativeTypeBuilder.h>
#include <Serialization/ContextsFwd.h>

#include <entt/entt.hpp>


namespace Rift
{
	namespace AST
	{
		template<typename, typename = void>
		struct BaseIdTraits;
		template<typename, typename = void>
		struct IdTraits;

		template<>
		struct BaseIdTraits<u32>
		{
			using Entity     = u32;
			using Index      = u32;
			using Version    = u16;
			using Difference = i64;

			static constexpr Entity indexMask   = 0xfffff;
			static constexpr Entity versionMask = 0xfff;
			static constexpr sizet indexShift   = 20u;
		};

		template<>
		struct BaseIdTraits<u64>
		{
			using Entity     = u64;
			using Index      = u32;
			using Version    = u32;
			using Difference = i64;

			static constexpr Entity indexMask   = 0xffffffff;
			static constexpr Entity versionMask = 0xffffffff;
			static constexpr sizet indexShift   = 32u;
		};

		template<IsEnum T>
		struct IdTraits<T> : public BaseIdTraits<UnderlyingType<T>>
		{
			using Parent     = BaseIdTraits<UnderlyingType<T>>;
			using Entity     = Parent::Entity;
			using Index      = Parent::Index;
			using Version    = Parent::Version;
			using Difference = Parent::Difference;


			static constexpr Index GetIndex(T id)
			{
				return Index(Entity(id) & Parent::indexMask);
			}

			static constexpr Version GetVersion(T id)
			{
				constexpr auto mask = Parent::versionMask << Parent::indexShift;
				return Version((Entity(id) & mask) >> Parent::indexShift);
			}

			static constexpr T Make(
			    Index index = Parent::indexMask, Version version = Parent::versionMask)
			{
				return T{(index & Parent::indexMask) | (Entity(version) << Parent::indexShift)};
			}
		};

		template<typename T>
			requires(std::is_class_v<T>)
		struct IdTraits<T> : IdTraits<typename T::EntityType>
		{};


		enum class Id : u32
		{};

		constexpr Id NoId = IdTraits<Id>::Make();
	}    // namespace AST

	namespace Serl
	{
		void Read(Serl::ReadContext& ct, AST::Id& val);
		void Write(Serl::WriteContext& ct, AST::Id val);
	}    // namespace Serl
}    // namespace Rift

REFLECT_NATIVE_TYPE(Rift::AST::Id);


namespace Rift::AST
{
	using VersionType = IdTraits<Id>::Version;

	template<typename... Type>
	using TExclude = entt::exclude_t<Type...>;

	template<typename... Type>
	using TTypeList = entt::type_list<Type...>;
}    // namespace Rift::AST


constexpr bool IsNone(Rift::AST::Id id)
{
	return id == Rift::AST::NoId;
}
