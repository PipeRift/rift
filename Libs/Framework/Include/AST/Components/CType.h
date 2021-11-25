// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <Misc/Guid.h>
#include <Object/Struct.h>


namespace Rift
{
	enum class TypeCategory : u8
	{
		None,
		Class,
		Struct,
		FunctionLibrary
	};

	struct CType : public Struct
	{
	public:
		using Super    = Struct;
		using ThisType = CType;
		using BuilderType =
		    Rift ::Refl ::TStructTypeBuilder<ThisType, Super, InitTypeFlags(Type_NoFlag)>;
		static Rift ::Refl ::StructType* GetType()
		{
			return Rift ::GetType<ThisType>();
		}
		void SerializeReflection(Rift ::Serl ::CommonContext& ct)
		{
			Super ::SerializeReflection(ct);
			__ReflSerializeProperty(ct, Rift ::Refl ::MetaCounter<0>{});
		}

	public:
		static Rift ::Refl ::StructType* InitType()
		{
			BuilderType builder{Rift ::Name{"CType"}};
			builder.onBuild = [](auto& builder) {
				__ReflReflectProperty(builder, Rift ::Refl ::MetaCounter<0>{});
				{
					{}
				}
			};
			builder.Initialize();
			return builder.GetType();
		}

	private:
		static constexpr Rift ::Refl ::MetaCounter<0> __refl_Counter(Rift ::Refl ::MetaCounter<0>);
		template<Rift ::u32 N>
		static void __ReflReflectProperty(BuilderType&, Rift ::Refl ::MetaCounter<N>)
		{}
		template<Rift ::u32 N>
		void __ReflSerializeProperty(Rift ::Serl ::CommonContext&, Rift ::Refl ::MetaCounter<N>)
		{}

	public:

		CType() {}
	};
}    // namespace Rift
