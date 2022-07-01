// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Id.h"
#include "Components/CTypeEditor.h"
#include "DockSpaceLayout.h"
#include "imgui.h"
#include "UI/Style.h"
#include "UI/Widgets.h"
#include "Utils/EditorStyle.h"
#include "Utils/ElementsPanel.h"
#include "Utils/FunctionGraphContextMenu.h"
#include "Utils/Nodes.h"
#include "Utils/Widgets.h"

#include <AST/Utils/Expressions.h>
#include <AST/Utils/Hierarchy.h>
#include <AST/Utils/TypeUtils.h>
#include <GLFW/glfw3.h>
#include <IconsFontAwesome5.h>
#include <Pipe/Core/EnumFlags.h>
#include <Pipe/ECS/Filtering.h>
#include <UI/UI.h>


namespace rift
{
	void DrawDetailsPanel(AST::Tree& ast, AST::Id typeId)
	{
		auto& editor = ast.Get<CTypeEditor>(typeId);

		if (!editor.showDetails)
		{
			return;
		}

		const String windowName = Strings::Format("Details##{}", typeId);
		if (UI::Begin(windowName.c_str(), &editor.showDetails)) {}
		UI::End();
	}
}    // namespace rift
