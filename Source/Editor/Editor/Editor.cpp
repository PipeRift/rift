// Copyright 2015-2019 Piperift - All rights reserved

#include "Editor.h"


void CodeEditor::Build()
{
	Super::Build();

	codeDockClass.ClassId = 2;

	bOpen = true;
	SetName(TX("Unit.cv"));
	windowFlags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_UnsavedDocument;
}

void CodeEditor::Tick(float deltaTime)
{
	if (bOpen)
	{
		BeginWindow();
		if (bWindowOpened)
		{
			// Create a DockSpace node where any window can be docked
			ImGuiID dockspace_id = ImGui::GetID("CodeEditor");
			ImGui::DockSpace(dockspace_id, ImVec2(0,0), 0, &codeDockClass);

			// Create Windows
			ImGui::SetNextWindowClass(&codeDockClass);
			ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);
			bool open = true;
			ImGui::Begin("Function Graph", &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
			ImGui::End();

			ImGui::SetNextWindowClass(&codeDockClass);
			ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);
			bool open2 = true;
			ImGui::Begin("Variables", &open2);
			ImGui::End();

			ImGui::SetNextWindowClass(&codeDockClass);
			ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);
			bool open3 = true;
			ImGui::Begin("Functions", &open3);
			ImGui::End();

			ImGui::SetNextWindowClass(&codeDockClass);
			ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);
			bool open4 = true;
			ImGui::Begin("Local Variables", &open4);
			ImGui::End();

			TickContent(deltaTime);
		}
		EndWindow();
	}
}

void CodeEditor::TickContent(float deltaTime)
{
	Super::TickContent(deltaTime);
}
