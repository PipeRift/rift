// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include <Assets/AssetPtr.h>
#include <Assets/TypeAsset.h>


namespace Rift
{
	class TypeAssetEditor;


	class TypePropertiesPanel
	{
		bool bOpen = true;

		TypeAssetEditor& editor;


	public:
		TypePropertiesPanel(TypeAssetEditor& editor);
		void Draw(struct DockSpaceLayout& layout);

		void DrawVariables();
		void DrawFunctions();

		void DrawVariable(StringView name);
	};
}    // namespace Rift
