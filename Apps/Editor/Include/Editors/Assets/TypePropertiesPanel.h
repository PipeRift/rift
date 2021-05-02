// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include <Assets/AssetPtr.h>
#include <Assets/TypeAsset.h>


namespace Rift
{
	class TypePropertiesPanel
	{
		bool bOpen = true;

		TAssetPtr<TypeAsset> asset;


	public:
		void Draw();

		void DrawVariables();
		void DrawFunctions();
	};
}    // namespace Rift
