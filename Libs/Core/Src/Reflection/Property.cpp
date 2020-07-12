// Copyright 2015-2020 Piperift - All rights reserved

#include "Reflection/Property.h"

#include "Reflection/Class.h"
#include "Reflection/Runtime/PropertyHandle.h"
#include "Reflection/Struct.h"


namespace VCLang::Refl
{
	void Property::SetDisplayName(const String& inDisplayName)
	{
		displayName = inDisplayName;

		// Remove bX preffix
		if (displayName.size() > 1 && CString::StartsWith(displayName, "b") &&
			isupper(displayName[1]))
		{
			CString::RemoveFromStart(displayName, 1);
			displayName[0] = eastl::CharToLower(displayName[0]);
		}
	}

	const Type* Property::GetInstanceType(const Ptr<BaseObject>& instance)
	{
		return instance->GetType();
	}
	const Type* Property::GetInstanceType(BaseStruct* instance)
	{
		return instance->GetType();
	}

}	 // namespace VCLang::Refl
