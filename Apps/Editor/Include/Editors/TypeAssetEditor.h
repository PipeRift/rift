// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Assets/TypePropertiesPanel.h"
#include "DockSpaceLayout.h"
#include "Editors/BaseEditor.h"
#include "NodeGraph/NodeGraphPanel.h"

#include <Assets/AssetPtr.h>
#include <Assets/TypeAsset.h>
#include <UI/UI.h>


namespace Rift
{
	class TypeAssetEditor : public BaseEditor
	{
		CLASS(TypeAssetEditor, BaseEditor)
	public:
		static const Name rightNode;
		static const Name centralNode;

	private:
		TAssetPtr<TypeAsset> asset;

		ImGuiID dockspaceID = 0;
		DockSpaceLayout layout;

		NodeGraphPanel nodeGraph{*this};
		TypePropertiesPanel properties{*this};
		bool pendingFocus = false;


	public:
		TypeAssetEditor();

		void SetAsset(TAssetPtr<TypeAsset> inAsset)
		{
			if (asset)
			{
				return;    // Can only assign an asset once
			}
			asset = inAsset;
		}

		void Draw(AST::Tree& ast);

		TAssetPtr<TypeAsset> GetAsset() const
		{
			return asset;
		}

		DockSpaceLayout& GetLayout()
		{
			return layout;
		}
		const DockSpaceLayout& GetLayout() const
		{
			return layout;
		}

		void MarkPendingFocus()
		{
			pendingFocus = true;
		}

		StringView GetWindowId() override
		{
			return asset.GetStrPath();
		}

		AST::Id GetNode()
		{
			return GetAsset()->declaration;
		}

	private:
		void CreateDockspace(const char* id);
	};
}    // namespace Rift
