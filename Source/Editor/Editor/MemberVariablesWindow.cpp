// Copyright 2015-2019 Piperift - All rights reserved

#include "MemberVariablesWindow.h"
#include "Editor.h"
#include "imgui/imgui_stl.h"


void MemberVariablesWindow::Build()
{
	Super::Build();

	SetName(TX("Variables"));

	Ptr<CodeEditor> codeEditor = GetOwner().Cast<CodeEditor>();
	windowClass = codeEditor->GetCodeDockClass();
	windowFlags |= ImGuiWindowFlags_NoCollapse;
}

void MemberVariablesWindow::TickContent(float)
{
	if (ImGui::CollapsingHeader(GetWindowID().c_str(), nullptr, ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Columns(3, "membervariables"); // 4-ways, with border
		ImGui::Separator();
		ImGui::Text("Type"); ImGui::NextColumn();
		ImGui::Text("Name"); ImGui::NextColumn();
		ImGui::Text("Default"); ImGui::NextColumn();
		ImGui::Separator();

		DrawTypeSelector("##ss22", "float"); ImGui::NextColumn();
		static String varName = "health";
		ImGui::InputText("##e2a", varName); ImGui::NextColumn();
		static float v = 0.1f;
		ImGui::InputFloat("##defaultfloat", &v, 0.f, 0.f, "%.2f"); ImGui::NextColumn();

		DrawTypeSelector("##s2s", "bool"); ImGui::NextColumn();
		static String varName2 = "alive";
		ImGui::InputText("##e2e", varName2); ImGui::NextColumn();
		static bool v2 = true;
		ImGui::Checkbox("##defa1ultbool", &v2); ImGui::NextColumn();
	}
}

void MemberVariablesWindow::DrawTypeSelector(const char* label, const char* selected)
{
	if (ImGui::BeginCombo(label, selected))
	{
		ImGui::Selectable("bool");
		ImGui::Selectable("u8");
		ImGui::Selectable("i32");
		ImGui::Selectable("u32");
		ImGui::EndCombo();
	}
}
