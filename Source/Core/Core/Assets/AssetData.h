// © 2019 Miguel Fernández Arce - All rights reserved

#pragma once

#include "CoreObject.h"
#include "AssetInfo.h"


class AssetData : public Object
{
	CLASS(AssetData, Object)

	AssetInfo info;

public:

	AssetData() : Super() {}

	/** Called after an asset is created. Internal usage only. */
	bool OnCreate(const AssetInfo& inInfo);

	/** Deserializes the asset. Internal usage only. */
	bool OnLoad(const AssetInfo& inInfo, Json& data);


	bool SaveToPath(const Name& path);

	bool Save();



protected:
	/** Called after the asset was loaded or created */
	virtual bool PostLoad() { return true; }
	virtual void BeforeDestroy() override { OnUnload(); }
	virtual void OnUnload() {}

	/** HELPERS */
public:

	const AssetInfo& GetInfo() const { return info; }

	const Name& GetMetaPath() const { return info.GetPath(); }
	const Name& GetRawPath() const { return info.GetPath(); }
};
