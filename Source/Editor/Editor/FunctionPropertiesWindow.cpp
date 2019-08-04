// Copyright 2015-2019 Piperift - All rights reserved

#include "FunctionPropertiesWindow.h"
#include "Editor.h"
#include "imgui/imgui.h"
#include "imgui/imgui_stl.h"


void FunctionPropertiesWindow::Build()
{
	Super::Build();

	SetName(TX("Function Details"));

	Ptr<CodeEditor> codeEditor = GetOwner().Cast<CodeEditor>();
	windowClass = codeEditor->GetCodeDockClass();
	windowFlags |= ImGuiWindowFlags_NoCollapse;
}

void FunctionPropertiesWindow::TickContent(float)
{
	if (ImGui::CollapsingHeader("Properties"))
	{
		if (ImGui::BeginCombo("Access", "Public"))
		{
			ImGui::Selectable("Public");
			ImGui::Selectable("Protected");
			ImGui::Selectable("Private");
			ImGui::EndCombo();
		}
	}

	ImGui::Spacing();
	ImGui::Spacing();
	bool bParametersOpen = ImGui::CollapsingHeader("Parameters", nullptr, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlapMode);
	{
		//Remove button
		ImGui::SameLine(ImGui::GetWindowWidth() - 30);
		if (ImGui::Button("+##params"))
		{
			bParametersOpen = false;
		}
	}
	if(bParametersOpen)
	{
		ImGui::Columns(4, "params"); // 4-ways, with border
		ImGui::Separator();
		ImGui::Text("Type"); ImGui::NextColumn();
		ImGui::Text("Name"); ImGui::NextColumn();
		ImGui::Text("Default"); ImGui::NextColumn();
		ImGui::Text(""); ImGui::NextColumn();
		ImGui::Separator();

		DrawTypeSelector("##s2s", "bool");  ImGui::NextColumn();
		static String varName = "alive";
		ImGui::InputText("##eg", varName); ImGui::NextColumn();
		static bool v2 = true;
		ImGui::SetNextItemWidth(-1);
		ImGui::Checkbox("##defaultbool", &v2); ImGui::NextColumn();
		ImGui::Button("X"); ImGui::NextColumn();

		ImGui::Columns(1);
	}

	ImGui::Spacing();
	ImGui::Spacing();
	bool bReturnsOpen = ImGui::CollapsingHeader("Returns", nullptr, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlapMode);
	{
		//Remove button
		ImGui::SameLine(ImGui::GetWindowWidth() - 30);
		if (ImGui::Button("+##returns"))
		{
			bParametersOpen = false;
		}
	}
	if (bReturnsOpen)
	{
		ImGui::Columns(4, "rets"); // 4-ways, with border
		ImGui::Separator();
		ImGui::Text("Type"); ImGui::NextColumn();
		ImGui::Text("Name"); ImGui::NextColumn();
		ImGui::Text("Default"); ImGui::NextColumn();
		ImGui::Text(""); ImGui::NextColumn();
		ImGui::Separator();

		DrawTypeSelector("##ss4", "bool");  ImGui::NextColumn();
		static String varName = "health";
		ImGui::InputText("##ei", varName); ImGui::NextColumn();
		static float v3 = 0.1f;
		ImGui::InputFloat("##defaultfloat", &v3, 0.f, 0.f, "%.2f"); ImGui::NextColumn();
		ImGui::Button("X"); ImGui::NextColumn();

		ImGui::Columns(1);
	}

	ImGui::Spacing();
	ImGui::Spacing();
	if (ImGui::CollapsingHeader("Local Variables", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Columns(4, "variables"); // 4-ways, with border
		ImGui::Separator();
		ImGui::Text("Type"); ImGui::NextColumn();
		ImGui::Text("Name"); ImGui::NextColumn();
		ImGui::Text("Default"); ImGui::NextColumn();
		ImGui::Text(""); ImGui::NextColumn();
		ImGui::Separator();

		DrawTypeSelector("##ss2", "float"); ImGui::NextColumn();
		static String varName = "health";
		ImGui::InputText("##ea", varName); ImGui::NextColumn();
		static float v = 0.1f;
		ImGui::InputFloat("##defaultfloat", &v, 0.f, 0.f, "%.2f"); ImGui::NextColumn();
		ImGui::Button("X"); ImGui::NextColumn();

		DrawTypeSelector("##ss", "bool"); ImGui::NextColumn();
		static String varName2 = "alive";
		ImGui::InputText("##ee", varName2); ImGui::NextColumn();
		static bool v2 = true;
		ImGui::Checkbox("##defaultbool", &v2); ImGui::NextColumn();
		ImGui::Button("X"); ImGui::NextColumn();

		ImGui::Columns(1);
	}
}

void FunctionPropertiesWindow::DrawTypeSelector(const char* label, const char* selected)
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
