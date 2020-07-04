// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "CoreEngine.h"
#include "CoreTypes.h"
#include "Strings/Name.h"
#include "TypeTraits.h"



class AssetInfo
{
protected:
	Name id;


public:
	AssetInfo() : id(Name::None())
	{
	}
	AssetInfo(Name id) : id(id)
	{
	}

	/**
	 * @returns true if this can never be pointed towards an asset
	 */
	const bool IsNull() const
	{
		return id.IsNone();
	}

	inline const Name& GetPath() const
	{
		return id;
	}
	inline const String& GetStrPath() const
	{
		return id.ToString();
	}

	bool Serialize(class Archive& ar, const char* name);

#if WITH_EDITOR
	static class Class* GetDetailsWidgetClass();
#endif
};

DEFINE_CLASS_TRAITS(AssetInfo, HasCustomSerialize = true, HasDetailsWidget = true);

DECLARE_REFLECTION_TYPE(AssetInfo);
