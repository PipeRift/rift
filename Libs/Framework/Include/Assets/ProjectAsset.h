// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "TypeAsset.h"

#include <Assets/AssetPtr.h>
#include <CoreObject.h>


namespace Rift
{
	/**
	 * ProjectAssets represents the asset that defines
	 */
	class ProjectAsset : public TypeAsset
	{
		CLASS(ProjectAsset, TypeAsset)

		PROP(String, version)
		String version;
	};
}    // namespace Rift
