// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include <Assets/AssetPtr.h>
#include <Assets/TypeAsset.h>


namespace Rift
{
	class AssetEditor;


	class TypePropertiesPanel
	{
		bool bOpen = true;

		AssetEditor& editor;


	public:
		TypePropertiesPanel(AssetEditor& editor);
		void Draw(struct DockSpaceLayout& layout);

		void DrawVariables();
		void DrawFunctions();

		void DrawVariable(StringView name);
	};
}    // namespace Rift
