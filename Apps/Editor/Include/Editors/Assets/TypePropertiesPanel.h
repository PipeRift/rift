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


		AST::Id selectedNode = AST::NoId;
		AST::Id renameNode   = AST::NoId;
		// Renaming uses this buffer to temporarely store the name being edited
		String renameBuffer;

		AST::Id pendingDelete = AST::NoId;


	public:
		TypePropertiesPanel(TypeAssetEditor& editor);
		void Draw(struct DockSpaceLayout& layout);

		void DrawVariables(AST::Tree& ast, AST::Id node);
		void DrawFunctions(AST::Tree& ast, AST::Id node);

		void DrawVariable(AST::Tree& ast, AST::Id id);
		void DrawFunction(AST::Tree& ast, AST::Id id);

		void DrawRenameInput(AST::Id id, struct CIdentifier* identifier);

		void ResetRename();
	};
}    // namespace Rift
