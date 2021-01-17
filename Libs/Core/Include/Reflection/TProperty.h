// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Property.h"
#include "ReflectionTags.h"
#include "Runtime/TPropertyHandle.h"
#include "Strings/Name.h"
#include "Type.h"

#include <functional>
#include <memory>


namespace Rift::Refl
{
	/**
	 * Static information about a property
	 */
	template <typename VariableT>
	class TProperty : public Property
	{
	public:
		using Access = std::function<VariableT*(void*)>;

	private:
		Access access;


	public:
		TProperty(Type* type, Name typeName, Name name, Access&& access, ReflectionTags tags)
			: Property(type, typeName, name, tags)
			, access(access)
		{}

		virtual std::shared_ptr<PropertyHandle> CreateHandle(
			const Ptr<BaseObject>& instance) const override
		{
			const Type* type = GetInstanceType(instance);
			if (type == GetContainerType() || type->IsChildOf(GetContainerType()))
			{
				return std::shared_ptr<PropertyHandle>(
					new TPropertyHandle<VariableT>(instance, this, access));
			}
			return {};
		}

		virtual std::shared_ptr<PropertyHandle> CreateHandle(BaseStruct* instance) const override
		{
			const Type* type = GetInstanceType(instance);
			if (type == GetContainerType() || type->IsChildOf(GetContainerType()))
			{
				return std::shared_ptr<PropertyHandle>(
					new TPropertyHandle<VariableT>(instance, this, access));
			}
			return {};
		}
	};
}	 // namespace Rift::Refl
