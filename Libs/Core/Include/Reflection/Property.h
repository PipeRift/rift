// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "ReflectionTags.h"
#include "Strings/Name.h"
#include "Strings/String.h"

#include <memory>

template <typename T>
class Ptr;
class BaseObject;


namespace Refl
{
	class Type;
	struct PropertyHandle;

	/**
	 * Static information about a property
	 */
	class Property
	{
	private:
		Type* typePtr;
		Name typeName;
		Name name;
		String displayName;
		ReflectionTags tags;


	public:
		Property() = delete;
		Property(Property&&) = delete;
		Property(const Property&) = delete;

	protected:
		Property(Type* typePtr, Name typeName, Name name, ReflectionTags tags)
			: typeName(typeName)
			, typePtr(typePtr)
			, name(name)
			, tags(tags)
		{
			SetDisplayName(name.ToString());
		}

	public:
		virtual ~Property() = default;

		const String& GetName() const
		{
			return name.ToString();
		}
		const String& GetDisplayName() const
		{
			return displayName;
		}

		bool HasTag(ReflectionTags tag) const
		{
			return HasAnyTags(tag);
		}
		bool HasAllTags(ReflectionTags inTags) const
		{
			return (tags & inTags) == inTags;
		}
		bool HasAnyTags(ReflectionTags inTags) const
		{
			return (tags & inTags) > 0;
		}

		Type* GetContainerType() const
		{
			return typePtr;
		}

		Name GetTypeName() const
		{
			return typeName;
		}

		virtual std::shared_ptr<PropertyHandle> CreateHandle(
			const Ptr<BaseObject>& instance) const = 0;
		virtual std::shared_ptr<PropertyHandle> CreateHandle(struct BaseStruct* instance) const = 0;

	private:
		void SetDisplayName(const String& inDisplayName);
	};
}	 // namespace Refl
