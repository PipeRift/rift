// Copyright 2015-2019 Piperift - All rights reserved

#include "MemberFunctionsWindow.h"
#include "Editor.h"
#include "imgui/imgui_internal.h"

void MemberFunctionsWindow::Build()
{
	Super::Build();

	SetName(TX("Functions"));

	Ptr<CodeEditor> codeEditor = GetOwner().Cast<CodeEditor>();
	windowClass = codeEditor->GetCodeDockClass();
	windowFlags |= ImGuiWindowFlags_NoCollapse;
}

void MemberFunctionsWindow::TickContent(float)
{
	ImGui::SetNextTreeNodeOpen(true, ImGuiCond_Always);
	if (ImGui::CollapsingHeader(GetWindowID().c_str(), nullptr, ImGuiTreeNodeFlags_DefaultOpen))
	{
		static ImGuiTextFilter filter;
		filter.Draw();
		ImGui::Indent();

		if (filter.PassFilter("bool Spawn"))
		{
			static bool selected = false;
			ImGui::Selectable("bool Spawn", &selected);
		}

		if (filter.PassFilter("void MoveToLocation"))
		{
			static bool selected = false;
			ImGui::Selectable("void MoveToLocation", &selected);
		}

		if (filter.PassFilter("bool IsAlive"))
		{
			static bool selected = false;
			ImGui::Selectable("bool IsAlive", &selected);
		}

		ImGui::Unindent();
	}
}
