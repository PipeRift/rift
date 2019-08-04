// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include "UI/Window.h"
#include "Core/Object/ObjectPtr.h"


namespace ImGui
{
	static ImGuiID GetWindowDockID(const char* windowName)
	{
		ImGuiWindow* window = ImGui::FindWindowByName(windowName);
		return window ? window->DockId : 0;
	}
};

namespace ImGuiUtil
{
	static void DockBuilderDockWindow(Ptr<::Window> window, ImGuiID dockId)
	{
		if (window)
		{
			const String logName = window->GetWindowID();
			ImGui::DockBuilderDockWindow(logName.c_str(), dockId);
		}
	}
};

