// Copyright 2015-2019 Piperift - All rights reserved

#include "Editor.h"


void Editor::Build()
{
	Super::Build();

	bOpen = true;
	SetName(TX("Class"));
	windowFlags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar;
}

void Editor::Tick(float deltaTime)
{
	if (bOpen)
	{
		BeginWindow();
		if (bWindowOpened)
		{
			// Create a DockSpace node where any window can be docked
			ImGuiID dockspace_id = ImGui::GetID("EditorDockspace");
			ImGui::DockSpace(dockspace_id);

			TickContent(deltaTime);
		}
		EndWindow();
	}
}

void Editor::TickContent(float deltaTime)
{
	Super::TickContent(deltaTime);
}
