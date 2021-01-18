// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "CoreEngine.h"
#include "Reflection/TProperty.h"
#include "Type.h"


namespace Rift::Refl
{
	class Struct : public Type
	{
	public:
		/** Equivalent to "new Type()" */
		virtual BaseStruct* New() const = 0;


		// NOTE: Most of the class comparison functions do actually
		// call Type to reduce complexity and code duplication.
		//
		// We can cast safely to Type since Structs only inherit Structs

		Struct* GetParent() const
		{
			return static_cast<Struct*>(parent);
		}

		void GetAllChildren(TArray<Struct*>& outChildren)
		{
			__GetAllChildren(reinterpret_cast<TArray<Type*>&>(outChildren));
		}

		Struct* FindChild(Name Name) const
		{
			return static_cast<Struct*>(__FindChild(Name));
		}

		bool IsA(Struct* other) const
		{
			return this == other;
		}

		/** Called internally to registry a property of a class */
		template <typename VariableT>
		void __RegistryProperty(
			Name name, std::function<VariableT*(void*)>&& access, ReflectionTags tags)
		{
			properties.Insert(name, new TProperty<VariableT>(this, GetReflectableName<VariableT>(),
										name, MoveTemp(access), tags));
		}
	};
}	 // namespace Rift::Refl