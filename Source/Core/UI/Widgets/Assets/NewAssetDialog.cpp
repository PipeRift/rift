// Copyright 2015-2019 Piperift - All rights reserved

#include "NewAssetDialog.h"
#include <imgui/imgui.h>
#include <imgui/imgui_stl.h>

void NewAssetDialog::Open()
{
	OpenDialog();
	newAssetName = "";
	finalPath.clear();
}

void NewAssetDialog::DrawFooter()
{
	// Name Input
	{
		ImGui::Text(currentPath.string().c_str());
		ImGui::SameLine();

		String label = CString::Printf("##%s Filename", title.c_str());
		ImGui::InputText(label.c_str(), newAssetName);
	}

	ImGui::Spacing();

	bool bFileExists = false;
	finalPath.clear();
	// Okay Button
	{
		const bool bCanSucceed = !newAssetName.empty();
		if (!bCanSucceed)
		{
			const ImVec4 lolight = ImGui::GetStyle().Colors[ImGuiCol_TextDisabled];
			ImGui::PushStyleColor(ImGuiCol_Button, lolight);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, lolight);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, lolight);
		}

		if (ImGui::Button("Ok", ImVec2(100, 20)) && bCanSucceed)
		{
			finalPath = currentPath / newAssetName.c_str();
			bFileExists = FileSystem::FileExists(finalPath);
		}

		if (!bCanSucceed)
		{
			ImGui::PopStyleColor(3);
		}
	}

	// Cancel Button
	ImGui::SetItemDefaultFocus();
	ImGui::SameLine();
	if (ImGui::Button("Cancel", ImVec2(100, 20)))
	{
		CloseDialog(EDialogResult::Cancel);
	}

	// Override popup
	if (bFileExists)
	{
		ImGui::OpenPopup("###override");
	}
	bool bCanOverride = false;
	if (ImGui::BeginPopupModal("Override existing asset?###override", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ImGui::Button("Yes", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();

			finalPath = currentPath / newAssetName.c_str();
			bCanOverride = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("No", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::SetItemDefaultFocus();
		ImGui::EndPopup();
	}

	if (!finalPath.empty() && (!bFileExists || bCanOverride))
	{
		CloseDialog(EDialogResult::Success);
	}
}
