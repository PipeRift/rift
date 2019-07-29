// Copyright 2015-2019 Piperift - All rights reserved

#include "BaseType.h"
#include "Property.h"


void BaseType::__GetAllChildren(TArray<BaseType*>& outChildren)
{
	// Yes, we loop two times children, but
	// also do one single allocation in each recursion
	outChildren.Append(children);
	for (auto* const child : children)
	{
		child->__GetAllChildren(outChildren);
	}
}

BaseType* BaseType::__FindChild(Name className) const
{
	if (className.IsNone())
		return nullptr;

	for (auto* const child : children)
	{
		if (child->GetName() == className)
		{
			return child;
		}
		else if (BaseType* found = child->__FindChild(className))
		{
			return found;
		}
	}
	return nullptr;
}

bool BaseType::IsChildOf(const BaseType* other) const
{
	if (!other || !parent)
		return false;

	const BaseType* current = parent;
	while (current)
	{
		if (other == current)
			return true;

		current = current->parent;
	}
	return false;
}

const Property* BaseType::FindProperty(Name propertyName) const
{
	const auto* prop = properties.Find(propertyName);
	return prop? prop->get() : nullptr;
}

void BaseType::GetOwnProperties(PropertyMap& outProperties) const
{
	outProperties.Resize(outProperties.Size() + properties.Size());
	for (const auto& prop : properties)
	{
		outProperties.Insert(prop.first, prop.second.get());
	}
}

void BaseType::GetAllProperties(PropertyMap& outProperties) const
{
	GetOwnProperties(outProperties);
	if (parent)
	{
		parent->GetAllProperties(outProperties);
	}
}
