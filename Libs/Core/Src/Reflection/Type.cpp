// Copyright 2015-2020 Piperift - All rights reserved

#include "Reflection/Type.h"

#include "Reflection/Property.h"


namespace Rift::Refl
{
	void Type::__GetAllChildren(TArray<Type*>& outChildren)
	{
		// Yes, we loop two times children, but
		// also do one single allocation in each recursion
		outChildren.Append(children);
		for (auto* const child : children)
		{
			child->__GetAllChildren(outChildren);
		}
	}

	Type* Type::__FindChild(Name className) const
	{
		if (className.IsNone())
			return nullptr;

		for (auto* const child : children)
		{
			if (child->GetName() == className)
			{
				return child;
			}
			else if (Type* found = child->__FindChild(className))
			{
				return found;
			}
		}
		return nullptr;
	}

	bool Type::IsChildOf(const Type* other) const
	{
		if (!other || !parent)
			return false;

		const Type* current = parent;
		while (current)
		{
			if (other == current)
				return true;

			current = current->parent;
		}
		return false;
	}

	const Property* Type::FindProperty(Name propertyName) const
	{
		const auto* prop = properties.Find(propertyName);
		return prop ? *prop : nullptr;
	}

	void Type::GetOwnProperties(PropertyMap& outProperties) const
	{
		outProperties.Resize(outProperties.Size() + properties.Size());
		for (const auto& prop : properties)
		{
			outProperties.Insert(prop.first, prop.second);
		}
	}

	void Type::GetAllProperties(PropertyMap& outProperties) const
	{
		if (parent)
		{
			parent->GetAllProperties(outProperties);
		}
		GetOwnProperties(outProperties);
	}
}	 // namespace Rift::Refl