// Copyright 2015-2019 Piperift - All rights reserved

#include "EditorManager.h"
#include <SDL_events.h>
#include <imgui/imgui_stl.h>
#include <imgui/imgui_impl_sdl.h>
#include <imgui/imgui_internal.h>

#include "Context.h"
#include "Core/Object/Object.h"
#include "Core/Assets/AssetPtr.h"
#include "UI/Widgets/Assets/NewAssetDialog.h"
#include "UI/Widgets/Assets/SelectAssetDialog.h"
#include "imgui/ImGuiUtil.h"


TMap<Name, ImFont*> EditorManager::fonts {};
const String EditorManager::configPath { FileSystem::ToString(FileSystem::GetConfigPath() / "ui.ini") };

void EditorManager::Construct()
{
	Super::Construct();

	// Change config file
	auto& io = ImGui::GetIO();
	io.IniFilename = configPath.c_str();

	ApplyStyle();

	newPage = CreateEditor<NewPageEditor>();
	CreateEditor<CodeEditor>();

	assetBrowser = Widget::CreateStandalone<AssetBrowser>(Self());
	assetBrowser->windowClass = mainDockClass;

	log = Widget::CreateStandalone<LogWindow>(Self());
	log->windowClass = mainDockClass;
}

void EditorManager::Tick(float deltaTime)
{
	ZoneScopedNC("Editor", 0x459bd1);
	DrawMainNavBar();

	TickDocking();

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (showDemoWindow)
		ImGui::ShowDemoWindow(&showDemoWindow);

	assetBrowser->OnTick(deltaTime);
	log->OnTick(deltaTime);

	editors.RemoveIf([](const auto & editor) { return !editor.IsValid(); });
	for (const auto& editor : editors)
	{
		editor->OnTick(deltaTime);
	}

	// Prepares the Draw List
	ImGui::Render();
}

void EditorManager::ApplyLayoutPreset()
{
	ImGui::DockBuilderRemoveNode(mainDock); // Clear out existing layout
	ImGui::DockBuilderAddNode(mainDock, ImGuiDockNodeFlags_DockSpace); // Add empty node
	ImVec2 size = ImGui::GetWindowSize();
	ImGui::DockBuilderSetNodeSize(mainDock, size);

	assetDock = mainDock; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.
	ImGuiID bottomDock = ImGui::DockBuilderSplitNode(assetDock, ImGuiDir_Down, 0.20f, nullptr, &assetDock);

	ImGuiUtil::DockBuilderDockWindow(log, bottomDock);
	ImGuiUtil::DockBuilderDockWindow(assetBrowser, bottomDock);

	for (const auto& editor : editors)
	{
		ImGuiUtil::DockBuilderDockWindow(editor, assetDock);
	}

	ImGui::DockBuilderFinish(mainDock);
}

void EditorManager::RestoreLayoutPreset()
{
	// Store the dock at which new page is
	const String ID = newPage->GetWindowID();
	assetDock = ImGui::GetWindowDockID(ID.c_str());
}

void EditorManager::TickDocking()
{
	// Create a full screen window and add a dockspace inside
	ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_MenuBar    | ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize   | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	bool b_dockspace_open = true;
	ImGui::Begin("EditorLayoutWindow", &b_dockspace_open, window_flags);
	{
		ImGui::PopStyleVar(2);

		// Dock space
		mainDock = ImGui::GetID("EditorLayout");

		if (ImGui::DockBuilderGetNode(mainDock) == nullptr)
		{
			ApplyLayoutPreset();
		}
		else
		{
			RestoreLayoutPreset();
		}

		static ImGuiDockNodeFlags opt_flags = ImGuiDockNodeFlags_PassthruCentralNode;
		ImGui::DockSpace(mainDock, ImVec2(0.0f, 0.0f), opt_flags, &mainDockClass);
	}
	ImGui::End();
}

void EditorManager::DrawMainNavBar()
{
	static NewAssetDialog saveSceneAsDialog{ "Save Scene" };
	static SelectAssetDialog openSceneDialog{ "Open Scene" };

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open File"))
			{
				openSceneDialog.Open();
			}
			if (ImGui::MenuItem("Save File", "CTRL+S"))
			{
			}
			if (ImGui::MenuItem("Save File as", "CTRL+S"))
			{
				saveSceneAsDialog.Open();
			}

			ImGui::Separator();
			if (ImGui::MenuItem("Exit")) {}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X")) {}
			if (ImGui::MenuItem("Copy", "CTRL+C")) {}
			if (ImGui::MenuItem("Paste", "CTRL+V")) {}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Asset Browser", (const char*)0, assetBrowser->IsOpenedPtr())) {}
			if (ImGui::MenuItem("Log", (const char*)0, log->IsOpenedPtr())) {}
			if (ImGui::MenuItem("Assets", (const char*)0, editors[0]->IsOpenedPtr())) {}
			if (ImGui::MenuItem("Demo", (const char*)0, &showDemoWindow)) {}

			ImGui::EndMenu();
		}

		if (ImGui::MenuItem("Play"))
		{
		}

		// Displays file name
		//const String sceneLabel = CString::Printf("%s###sceneLabel", GetContext()->GetActiveScene().GetSPath().c_str());
		//ImGui::MenuItem(sceneLabel.c_str(), nullptr, false, false);

		ImGui::EndMainMenuBar();
	}

	// Draw modals
	if (openSceneDialog.Draw() == EDialogResult::Success)
	{
		const Name fileName = FileSystem::ToString(openSceneDialog.selectedAsset);
		//
	}

	if (saveSceneAsDialog.Draw() == EDialogResult::Success)
	{
		const Name newAssetId = FileSystem::ToString(saveSceneAsDialog.finalPath);
		//
	}
}

void EditorManager::AddFont(Name name, Path path, u8 size)
{
	const String sPath = FileSystem::ToString(path);

	if (!FileSystem::FileExists(path))
	{
		Log::Error("Font doesn't exist in folder '%s'", sPath.c_str());
		return;
	}

	if (name.IsNone())
	{
		Log::Error("No name provided for font '%s'", sPath.c_str());
		return;
	}

	auto& io = ImGui::GetIO();
	ImFont* loadedFont = io.Fonts->AddFontFromFileTTF(sPath.c_str(), size);

	if (loadedFont)
	{
		fonts.Insert(name, loadedFont);
	}
	else
	{
		Log::Error("Font '%s' could not be loaded at '%s'", name.ToString().c_str(), sPath.c_str());
	}
}

void EditorManager::PushFont(Name name)
{
	ImFont** font = fonts.Find(name);
	if (font && *font)
	{
		ImGui::PushFont(*font);
	}
	else
	{
		Log::Error("Font '%s' not found or loaded", name.ToString().c_str());
	}
}

void EditorManager::PopFont()
{
	ImGui::PopFont();
}

void EditorManager::ApplyStyle()
{
	fonts.SetEmptyKey("");
	AddFont("Karla", FileSystem::GetAssetsPath() / "Fonts/karla_regular.ttf");
	AddFont("KarlaBold", FileSystem::GetAssetsPath() / "Fonts/karla_bold.ttf");
	AddFont("KarlaItalic", FileSystem::GetAssetsPath() / "Fonts/karla_italic.ttf");
	AddFont("KarlaBoldItalic", FileSystem::GetAssetsPath() / "Fonts/karla_bold_italic.ttf");

	// Default style
	ImGui::StyleColorsDark();


	auto& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;
	colors[ImGuiCol_Text] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.94f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.08f, 0.08f, 0.09f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.23f, 0.22f, 0.22f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.64f, 0.64f, 0.64f, 0.54f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.98f, 0.77f, 0.26f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.39f, 0.25f, 0.13f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.60f, 0.38f, 0.20f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.05f, 0.07f, 0.09f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.07f, 0.09f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.98f, 0.58f, 0.26f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.88f, 0.53f, 0.24f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.98f, 0.58f, 0.26f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.49f, 0.49f, 0.49f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.98f, 0.58f, 0.26f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.98f, 0.58f, 0.26f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.98f, 0.58f, 0.26f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.98f, 0.58f, 0.26f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.75f, 0.39f, 0.10f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.75f, 0.39f, 0.10f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.98f, 0.58f, 0.26f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.98f, 0.58f, 0.26f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.98f, 0.58f, 0.26f, 0.95f);
	colors[ImGuiCol_Tab] = ImVec4(0.40f, 0.25f, 0.13f, 0.86f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.95f, 0.56f, 0.25f, 0.80f);
	colors[ImGuiCol_TabActive] = ImVec4(0.62f, 0.39f, 0.21f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.50f, 0.31f, 0.17f, 0.97f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.62f, 0.39f, 0.21f, 1.00f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.98f, 0.58f, 0.26f, 0.70f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.98f, 0.77f, 0.26f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.98f, 0.77f, 0.26f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

	style.WindowRounding = 1;
	style.TabRounding = 1;
	style.ScrollbarRounding = 2;
}
