// Copyright 2015-2019 Piperift - All rights reserved

#include "SelectAssetDialog.h"
#include <imgui/imgui.h>
#include <imgui/imgui_stl.h>


void SelectAssetDialog::Open()
{
	OpenDialog();
	selectedAsset.clear();
}

void SelectAssetDialog::DrawFooter()
{
	if (!bShowButtons)
		return;

	// Name Input
	ImGui::Text(selectedAsset.string().c_str());

	ImGui::Spacing();

	// Okay Button
	{
		const bool bCanSucceed = !selectedAsset.empty();
		if (!bCanSucceed)
		{
			const ImVec4 lolight = ImGui::GetStyle().Colors[ImGuiCol_TextDisabled];
			ImGui::PushStyleColor(ImGuiCol_Button, lolight);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, lolight);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, lolight);
		}

		if (ImGui::Button("OK", ImVec2(100, 20)) && bCanSucceed)
		{
			CloseDialog(EDialogResult::Success);
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
}
