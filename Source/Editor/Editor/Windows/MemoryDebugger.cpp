// Copyright 2015-2019 Piperift - All rights reserved

#include "MemoryDebugger.h"
#include "Core/Memory/Allocator.h"
#include "imgui/imgui_stl.h"


#if WITH_EDITOR

void MemoryDebugger::Build()
{
	Super::Build();
	bOpen = true;
	SetName(TX("Memory"));
	windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
}

void MemoryDebugger::Tick(float)
{
	if (bOpen)
	{
		BeginWindow();
		{
			ImGui::Text(Memory::GetAllocator()->GetName().ToString());
			ImGui::SameLine();
			ImGui::Text(" -- %i kb", (Memory::GetAllocator()->GetSize() / 1024));
			ImGui::Separator();
			ImGui::Text(Memory::GetObjectsAllocator()->GetName().ToString());
			ImGui::SameLine();
			ImGui::Text("-- %i kb", (Memory::GetObjectsAllocator()->GetSize() / 1024));
			ImGui::Separator();
			ImGui::Text(Memory::GetAssetsAllocator()->GetName().ToString());
			ImGui::SameLine();
			ImGui::Text(" -- %i kb", (Memory::GetAssetsAllocator()->GetSize() / 1024));
			ImGui::Separator();
			ImGui::Text(Memory::GetFrameAllocator()->GetName().ToString());
			ImGui::SameLine();
			ImGui::Text("  -- %i kb", (Memory::GetFrameAllocator()->GetSize() / 1024));
		}
		EndWindow();
	}
}

#endif
