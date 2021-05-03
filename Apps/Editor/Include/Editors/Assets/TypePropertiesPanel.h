// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include <Assets/AssetPtr.h>
#include <Assets/TypeAsset.h>


namespace Rift
{
	class TypePropertiesPanel
	{
		bool bOpen = true;

		class AssetEditor* owner;


	public:
		TypePropertiesPanel() {}
		void Draw(StringView baseId, struct DockSpaceLayout& layout);

		void DrawVariables();
		void DrawFunctions();

		void DrawVariable(StringView name);

		AST::Id GetOwnerNode() const;
	};
}    // namespace Rift
