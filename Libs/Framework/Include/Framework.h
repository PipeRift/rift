
#pragma once

#include <Assets/AssetManager.h>

namespace VCLang
{
	class Framework
	{
		GlobalPtr<AssetManager> assetManager;

	public:

		Framework()
		{
			assetManager = Create<AssetManager>();
		}

		Ptr<AssetManager> GetAssetManager() const { return assetManager; }
	};
}
