// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include "PropertyHandle.h"
#include "Core/Reflection/Property.h"


/**
 * Base logic implementation for templated property handles.
 * Use TPropertyHandle instead
 */
template<typename VarType>
struct TPropertyHandle : public PropertyHandle
{
	using Access = eastl::function<VarType*(BaseStruct*)>;

	Access access;


public:
	TPropertyHandle() : PropertyHandle({}, nullptr) {}
	TPropertyHandle(const Ptr<BaseObject>& instance, const Property* prop, const Access& access)
		: PropertyHandle(instance, prop), access{access}
	{}

	TPropertyHandle(BaseStruct* instance, const Property* prop, const Access& access)
		: PropertyHandle(instance, prop), access{ access }
	{}

	VarType* GetValuePtr() const
	{
		if (IsValid())
		{
			return access(GetInstance());
		}
		return nullptr;
	}
	virtual void* GetRawValuePtr() const override { return GetValuePtr(); }

	bool GetValue(VarType& value) const
	{
		VarType* const valuePtr = GetValuePtr();
		if (valuePtr)
			value = *valuePtr;

		return valuePtr;
	}

	bool SetValue(const VarType& value) const
	{
		VarType* const valuePtr = GetValuePtr();
		if (valuePtr)
			*valuePtr = value;

		return valuePtr;
	}

#if WITH_EDITOR
	FORCEINLINE virtual Class* GetClassDefinedWidgetClass() override
	{
		if constexpr(ClassTraits<VarType>::HasDetailsWidget)
			return VarType::GetDetailsWidgetClass();
		else
			return nullptr;
	}
#endif
};
