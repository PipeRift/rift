 // Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include "CoreTypes.h"

#include "Core/Object/BaseStruct.h"
#include "Core/Object/BaseObject.h"
#include "Core/Object/ObjectPtr.h"
#include "../Property.h"


#if WITH_EDITOR
class PropertyWidget;
#endif


struct PropertyHandle {
protected:

	BaseStruct* const instance;
	Ptr<BaseObject> objInstance;
	const Property* const prop;


	PropertyHandle(const Ptr<BaseObject>& objInstance, const Property* prop) : objInstance{ objInstance }, prop{ prop }, instance{nullptr} {}
	PropertyHandle(BaseStruct* instance, const Property* prop) : instance{ instance }, prop{ prop } {}

public:

	virtual ~PropertyHandle() {}


	FORCEINLINE String GetName() const
	{
		if (prop)
			return prop->GetDisplayName();
		return "Invalid";
	}

	FORCEINLINE bool HasTag(ReflectionTags tag) const {
		return prop ? prop->HasTag(tag) : false;
	}

	FORCEINLINE BaseStruct* GetInstance() const {
		return UsesObjectPtr()? *objInstance : instance;
	}

	bool IsValid() const {
		if (UsesObjectPtr())
		{
			return objInstance && prop != nullptr;
		}
		else
		{
			return instance && prop != nullptr;
		}
	}
	operator bool() const { return IsValid(); }

	bool UsesObjectPtr() const {
		return objInstance.IsValid();
	}

	FORCEINLINE virtual Class* GetClassDefinedWidgetClass() {
		return nullptr;
	}

	virtual void* GetRawValuePtr() const = 0;
};
