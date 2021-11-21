// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <Platform/Platform.h>
#include <Reflection/Registry/NativeTypeBuilder.h>
#include <Serialization/ContextsFwd.h>
#include <Templates/TypeList.h>


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
		};

		template<typename T>
			requires(std::is_class_v<T>)
		struct IdTraits<T> : IdTraits<typename T::EntityType>
		{};


		enum class Id : u32
		{};

		static constexpr Id MakeId(IdTraits<Id>::Index index = IdTraits<Id>::indexMask,
		    IdTraits<Id>::Version version                    = IdTraits<Id>::versionMask)
		{
			return Id{(index & IdTraits<Id>::indexMask)
			          | (IdTraits<Id>::Entity(version) << IdTraits<Id>::indexShift)};
		}

		constexpr Id NoId = MakeId();

		static constexpr IdTraits<Id>::Index GetIndex(Id id)
		{
			return IdTraits<Id>::Index{IdTraits<Id>::Entity(id) & IdTraits<Id>::indexMask};
		}

		static constexpr IdTraits<Id>::Version GetVersion(Id id)
		{
			constexpr auto mask = IdTraits<Id>::versionMask << IdTraits<Id>::indexShift;
			return IdTraits<Id>::Version{
			    (IdTraits<Id>::Entity(id) & mask) >> IdTraits<Id>::indexShift};
		}
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
	using TExclude = TTypeList<Type...>;

}    // namespace Rift::AST


constexpr bool IsNone(Rift::AST::Id id)
{
	return id == Rift::AST::NoId;
}
