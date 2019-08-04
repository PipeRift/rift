// Copyright 2015-2019 Piperift - All rights reserved

#include "Editor.h"
#include <imgui/imgui_internal.h>

#include "imgui/ImGuiUtil.h"


u32 CodeEditor::codeEditorCount{ 0 };


void NewPageEditor::Build()
{
	Super::Build();
	SetName(TX("Start Page"));
}


void CodeEditor::Build()
{
	Super::Build();

	codeDockClass.ClassId = 2 + codeEditorCount;
	++codeEditorCount;

	bOpen = true;
	SetName(TX("Unit.cv"));
	windowFlags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_UnsavedDocument;

	// Initialize windows
	variables      = W(MemberVariablesWindow);
	functions      = W(MemberFunctionsWindow);
	functionProperties = W(FunctionPropertiesWindow);
	functionGraph  = W(FunctionGraphWindow);
}

void CodeEditor::Tick(float deltaTime)
{
	if (bOpen)
	{
		BeginWindow();
		if (bWindowOpened)
		{
			codeDock = ImGui::GetID("CodeEditor");

			if (ImGui::DockBuilderGetNode(codeDock) == nullptr)
			{
				ApplyLayoutPreset();
			}

			ImGui::DockSpace(codeDock, ImVec2(0, 0), ImGuiDockNodeFlags_NoCloseButton | ImGuiDockNodeFlags_AutoHideTabBar, &codeDockClass);

			TickContent(deltaTime);
		}
		EndWindow();
	}
}

void CodeEditor::ApplyLayoutPreset()
{
	ImGui::DockBuilderRemoveNode(codeDock); // Clear out existing layout
	ImGui::DockBuilderAddNode(codeDock, ImGuiDockNodeFlags_DockSpace); // Add empty node
	ImVec2 size = ImGui::GetWindowSize();
	ImGui::DockBuilderSetNodeSize(codeDock, size);

	ImGuiID centralDock = codeDock; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.
	ImGuiID leftDock = ImGui::DockBuilderSplitNode(centralDock, ImGuiDir_Left, 0.15f, nullptr, &centralDock);
	ImGuiID rightDock = ImGui::DockBuilderSplitNode(centralDock, ImGuiDir_Right, 0.20f, nullptr, &centralDock);

	ImGuiID leftTopDock;
	ImGuiID leftBottomDock = ImGui::DockBuilderSplitNode(leftDock, ImGuiDir_Down, 0.70f, nullptr, &leftTopDock);

	ImGuiUtil::DockBuilderDockWindow(variables, leftTopDock);
	ImGuiUtil::DockBuilderDockWindow(functions, leftBottomDock);
	ImGuiUtil::DockBuilderDockWindow(functionGraph, centralDock);
	ImGuiUtil::DockBuilderDockWindow(functionProperties, rightDock);

	ImGui::DockBuilderFinish(codeDock);
}
