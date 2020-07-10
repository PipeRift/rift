// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Containers/Array.h"
#include "Containers/Map.h"
#include "CoreEngine.h"
#include "CoreTypes.h"
#include "Reflection/Property.h"
#include "Reflection/ReflectionTags.h"


namespace Refl
{
	using PropertyMap = TMap<Name, Property*>;

	/** Smallest reflection type that contains all basic class or struct data */
	class Type
	{
	protected:
		Name name;
		ReflectionTags tags;

		Type* parent;
		TArray<Type*> children;

		PropertyMap properties{};


	public:
		Type() : tags{}, parent{nullptr} {}
		Type(const Type&) = delete;
		Type& operator=(const Type&) = delete;
		virtual ~Type()
		{
			// TODO: Destroy properties
		}


		/** Type */
		const Name& GetName() const
		{
			return name;
		}
		const String& GetSName() const
		{
			return GetName().ToString();
		}

		bool HasTag(ReflectionTags tag) const
		{
			return (tags & tag) > 0;
		}

	protected:
		void __GetAllChildren(TArray<Type*>& outChildren);
		Type* __FindChild(Name className) const;

	public:
		bool IsChildOf(const Type* other) const;

		template <typename T>
		bool IsChildOf() const
		{
			return IsChildOf(T::StaticType());
		}

		bool IsParentOf(const Type* other) const
		{
			return other->IsChildOf(this);
		}


		/** Properties */

		const Property* FindProperty(Name propertyName) const;
		void GetOwnProperties(PropertyMap& outProperties) const;
		void GetAllProperties(PropertyMap& outProperties) const;


		/** REGISTRY */

		/** Registry a class */
		void __Registry(Name inName)
		{
			name = inName;
		}

		/** Registry a class with a parent */
		template <typename Super>
		void __Registry(Name inName)
		{
			parent = Super::StaticType();
			parent->__RegistryChild(this);
			__Registry(inName);
		}

		/** Called internally to registry class tags */
		void __RegistryTags(ReflectionTags inTags)
		{
			tags = inTags;
		}

	private:
		void __RegistryChild(Type* child)
		{
			children.Add(child);
		}
	};
}	 // namespace Refl
