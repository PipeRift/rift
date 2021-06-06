// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <Platform/Platform.h>
#include <Strings/StringView.h>


namespace Rift
{
	class TypeAssetEditor;

	class NodeGraphPanel
	{
		TypeAssetEditor& editor;

		AST::Id declaration = AST::NoId;


	public:
		NodeGraphPanel(TypeAssetEditor& editor);
		void Draw(struct DockSpaceLayout& layout);


		void SetDeclaration(AST::Id id)
		{
			declaration = id;
		}
	};
}    // namespace Rift
