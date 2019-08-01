// Copyright 2015-2019 Piperift - All rights reserved

#include "AssetDialog.h"
#include <imgui/imgui.h>
#include <imgui/imgui_stl.h>


void AssetDialog::OpenDialog()
{
	bPendingOpen = true;
}

void AssetDialog::CloseDialog(EDialogResult reason)
{
	ImGui::CloseCurrentPopup();
	result = reason;
}

EDialogResult AssetDialog::Draw()
{
	if (bPendingOpen)
	{
		ImGui::OpenPopup(title.c_str());
		result = EDialogResult::None;
		bPendingOpen = false;
	}

	bool bClose; // Adds a close button
	if (ImGui::BeginPopupModal(title.c_str(), &bClose, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (bDirty)
		{
			CacheCurrentDirectory();
		}

		DrawHeader();
		DrawContent();
		DrawFooter();

		ImGui::EndPopup();

		if (!navigatePath.empty())
		{
			currentPath = navigatePath;
			navigatePath.clear();
			ResetCache();
		}
	}

	if (result != EDialogResult::None)
	{
		// Reset on return
		EDialogResult c = result;
		result = EDialogResult::None;
		return c;
	}
	return EDialogResult::None;
}

void AssetDialog::DrawHeader()
{
	if (ImGui::Button("Assets"))
	{
		navigatePath = "/";
	}

	ImGui::SameLine();

	String path = FileSystem::ToString(currentPath);
	ImGui::Text(path.c_str());
}

void AssetDialog::DrawContent()
{
	// Directories
	{
		ImGui::BeginChild("Directories", ImVec2(200, 300), true, ImGuiWindowFlags_HorizontalScrollbar);
		if (ImGui::TreeNode("Roots"))
		{
			for (auto& p : roots)
			{
				if (ImGui::Selectable(p.label.c_str(), false, 0, ImVec2(ImGui::GetWindowContentRegionWidth(), 0)))
				{
					navigatePath = p.root;
				}
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNodeEx("Directories", ImGuiTreeNodeFlags_DefaultOpen))
		{
			const Path absCurrentPath = FileSystem::GetAssetsPath() / currentPath;

			if (FileSystem::IsAssetPath(absCurrentPath.parent_path()) &&
				ImGui::Selectable("..", false, 0, ImVec2(ImGui::GetWindowContentRegionWidth(), 0)))
			{
				const Path parentPath = currentPath.parent_path();
				navigatePath = parentPath.empty()? "/" : parentPath;
			}
			for (auto& p : directories)
			{
				if (ImGui::Selectable(p.filename().string().c_str(), false, 0, ImVec2(ImGui::GetWindowContentRegionWidth(), 0)))
				{
					navigatePath = p;
				}
			}
			ImGui::TreePop();
		}
		ImGui::EndChild();
	}
	ImGui::SameLine();

	// Files
	{
		ImGui::BeginChild("Files", ImVec2(400, 300), true, ImGuiWindowFlags_HorizontalScrollbar);
		ImGui::Columns(3);
		ImGui::SetColumnWidth(1, 60.f);

		ImGui::PushItemWidth(-1);
		const String name = CString::Printf("%s Name", sorter.GetOrderSymbol(EDialogSortType::Name));
		if (ImGui::Selectable(name.c_str()))
		{
			if (sorter.type != EDialogSortType::Name)
			{
				sorter.type = EDialogSortType::Name;
				sorter.order = EDialogSortOrder::SortDown;
			}
			else
			{
				sorter.SwitchToNextOrder();
			}
			ResetCache();
		}
		ImGui::NextColumn();

		const String size = CString::Printf("%s Size", sorter.GetOrderSymbol(EDialogSortType::Size));
		if (ImGui::Selectable(size.c_str()))
		{
			if (sorter.type != EDialogSortType::Size)
			{
				sorter.type = EDialogSortType::Size;
				sorter.order = EDialogSortOrder::SortDown;
			}
			else
			{
				sorter.SwitchToNextOrder();
			}
			ResetCache();
		}
		ImGui::NextColumn();

		const String date = CString::Printf("%s Date", sorter.GetOrderSymbol(EDialogSortType::Date));
		if (ImGui::Selectable(date.c_str()))
		{
			if (sorter.type != EDialogSortType::Date)
			{
				sorter.type = EDialogSortType::Date;
				sorter.order = EDialogSortOrder::SortDown;
			}
			else
			{
				sorter.SwitchToNextOrder();
			}
			ResetCache();
		}
		ImGui::NextColumn();
		ImGui::Separator();

		for (auto& file : files)
		{
			const String filename = FileSystem::ToString(file.path.stem());
			// Remove .meta
			//CString::RemoveFromEnd(filename, 5);

			const String label = CString::Printf("##%s", filename.c_str());

			if (ImGui::Selectable(label.c_str(), file.bSelected, ImGuiSelectableFlags_SpanAllColumns))
			{
				for (auto& f : files)
				{
					f.bSelected = false;
				}
				file.bSelected = true;

				OnFileSelected(file);
			}
			ImGui::SameLine();
			ImGui::Text(filename.c_str());
			ImGui::NextColumn();
			String sizeValue = CString::ParseMemorySize(size_t(file.size));
			ImGui::Text(sizeValue.c_str());
			ImGui::NextColumn();
			const String dateStr = file.dateTime.ToLocal().ToString(TX("%d/%m/%Y, %H:%M"));
			ImGui::Text(dateStr.c_str());
			ImGui::NextColumn();
		}
		ImGui::EndChild();
	}
}

void AssetDialog::CacheCurrentDirectory()
{
	CacheRoots();

	if (currentPath.empty())
		return;

	Path absCurrentPath;
	if (CString::Equals(FileSystem::ToString(currentPath), '/'))
	{
		absCurrentPath = FileSystem::GetAssetsPath();
	}
	else
	{
		absCurrentPath = FileSystem::GetAssetsPath() / currentPath;
	}

	for (auto& p : FileSystem::DirectoryIterator(absCurrentPath))
	{
		Path path = p.path();
		FileSystem::RelativeToAssetsPath(path);

		if (p.is_directory())
		{
			directories.Add(path);
		}
		else if(FileSystem::IsAssetFilePath(p))
		{
			auto status = p.status();
			auto lastWrite = fs::last_write_time(p);
			DateTime lastWriteDate = DateTime::CastClock(lastWrite);

			u64 size = fs::file_size(p);

			// Add raw file size if any
			Path rawPath = FileSystem::FindRawFile(path);
			if (fs::exists(rawPath))
			{
				size += fs::file_size(rawPath);
			}

			files.Add({ path, size, lastWriteDate });
		}
	}

	// Sort files & directories
	if (sorter.order != EDialogSortOrder::Unsorted)
	{
		std::sort(files.begin(), files.end(), sorter);
	}
	std::sort(directories.begin(), directories.end());
	bDirty = false;
}

void AssetDialog::CacheRoots() { roots.Add({ "/", "(root)" }); }

bool AssetDialog::GetSelectedFile(AssetDialogFile& selected)
{
	for (auto& file : files)
	{
		if (file.bSelected)
		{
			selected = file;
			return true;
		}
	}
	return false;
}
