// © 2019 Miguel Fernández Arce - All rights reserved

#pragma once

#include "CoreEngine.h"
#include "CoreTypes.h"
#include "Files/FileSystem.h"
#include "Strings/Name.h"
#include "TypeTraits.h"



class AssetInfo
{
public:
	AssetInfo() : id(Name::None())
	{
	}
	AssetInfo(Name id) : id(id)
	{
	}

protected:
	Name id;

public:
	/**
	 * @returns true if this can never be pointed towards an asset
	 */
	const bool IsNull() const
	{
		return id.IsNone() || !FileSystem::IsAssetPath(GetSPath());
	}

	inline const Name& GetPath() const
	{
		return id;
	}
	inline const String& GetSPath() const
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
