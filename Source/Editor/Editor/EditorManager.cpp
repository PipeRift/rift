// Copyright 2015-2019 Piperift - All rights reserved

#include "EditorManager.h"
#include <SDL_events.h>
#include <imgui/imgui_stl.h>
#include <imgui/imgui_impl_sdl.h>
#include <imgui/imgui_internal.h>

#include "World.h"
#include "Core/Object/Object.h"
#include "Core/Assets/AssetPtr.h"
#include "Assets/Scene.h"
#include "Widgets/Assets/NewAssetDialog.h"
#include "Widgets/Assets/SelectAssetDialog.h"


#if WITH_EDITOR

TMap<Name, ImFont*> EditorManager::fonts {};
const String EditorManager::configPath { FileSystem::ToString(FileSystem::GetConfigPath() / "ui.ini") };

void EditorManager::Construct()
{
	Super::Construct();

	fonts.SetEmptyKey("");

	// Change config file
	auto& io = ImGui::GetIO();
	io.IniFilename = configPath.c_str();

	// Setup style
	AddFont("Karla", FileSystem::GetAssetsPath() / "Fonts/karla_regular.ttf");
	AddFont("KarlaBold", FileSystem::GetAssetsPath() / "Fonts/karla_bold.ttf");
	AddFont("KarlaItalic", FileSystem::GetAssetsPath() / "Fonts/karla_italic.ttf");
	AddFont("KarlaBoldItalic", FileSystem::GetAssetsPath() / "Fonts/karla_bold_italic.ttf");
	ImGui::StyleColorsDark();
	ApplyStyle();

	sceneEditor = CreateEditor<SceneEditor>();

	assetBrowser = Widget::CreateStandalone<AssetBrowser>(Self());
	log = Widget::CreateStandalone<LogWindow>(Self());
}

void EditorManager::Tick(float deltaTime)
{
	ZoneScopedNC("Editor", 0x459bd1);
	TickMainNavBar();
	TickDocking();

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (showDemoWindow)
		ImGui::ShowDemoWindow(&showDemoWindow);

	editors.RemoveIf([](const auto & editor) { return !editor.IsValid(); });
	for (const auto& editor : editors)
	{
		editor->Tick(deltaTime);
	}

	assetBrowser->OnTick(deltaTime);
	log->OnTick(deltaTime);

	// Prepares the Draw List
	ImGui::Render();
}

void EditorManager::TickDocking()
{
	static bool opt_fullscreen_persistant = true;
	static ImGuiDockNodeFlags opt_flags = ImGuiDockNodeFlags_PassthruCentralNode;
	bool opt_fullscreen = opt_fullscreen_persistant;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	// When using ImGuiDockNodeFlags_PassthruDockspace, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (opt_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	bool b_dockspace_open = true;
	ImGui::Begin("EditorLayoutWindow", &b_dockspace_open, window_flags);
	ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(1);

	// Dock space
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("EditorLayout");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), opt_flags);
	}
	ImGui::End();
}

void EditorManager::TickMainNavBar()
{
	static NewAssetDialog saveSceneAsDialog{ "Save Scene" };
	static SelectAssetDialog openSceneDialog{ "Open Scene" };

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open Scene"))
			{
				openSceneDialog.Open();
			}
			if (ImGui::MenuItem("Save Scene", "CTRL+S"))
			{
				if (TAssetPtr<Scene> scene = GetWorld()->GetActiveScene())
				{
					scene->SaveScene(GetWorld());
					scene->Save();
				}
			}
			if (ImGui::MenuItem("Save Scene as", "CTRL+S"))
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

			for (const auto& editor : editors)
			{
				editor->ExpandViewsMenu();
			}

			ImGui::EndMenu();
		}

		auto world = GetWorld();
		if (world->IsEditor())
		{
			if (ImGui::MenuItem("Play"))
			{
				world->BeginPlay();
			}
		}
		else if (world->IsPIE())
		{
			if (ImGui::MenuItem("Stop"))
			{
				world->EndPlay();
			}
		}

		// Displays scene name
		const String sceneLabel = CString::Printf("%s###sceneLabel", GetWorld()->GetActiveScene().GetSPath().c_str());
		ImGui::MenuItem(sceneLabel.c_str(), nullptr, false, false);

		ImGui::EndMainMenuBar();
	}

	// Draw modals
	if (openSceneDialog.Draw() == EDialogResult::Success)
	{
		const Name sceneName = FileSystem::ToString(openSceneDialog.selectedAsset);
		GetWorld()->LoadScene({ sceneName });
	}

	if (saveSceneAsDialog.Draw() == EDialogResult::Success)
	{
		if (TAssetPtr<Scene>& scene = GetWorld()->GetActiveScene())
		{
			const Name newAssetId = FileSystem::ToString(saveSceneAsDialog.finalPath);

			// Save to specified asset
			scene->SaveScene(GetWorld());
			scene->SaveToPath(newAssetId);

			// Replace active scene
			scene = { newAssetId };
			scene.Load();
		}
	}
}

void EditorManager::OnSDLEvent(SDL_Event* ev)
{
	ImGui_ImplSDL2_ProcessEvent(ev);
}

void EditorManager::AddFont(Name name, Path path, u8 size)
{
	String sPath = FileSystem::ToString(path);

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
	auto& style = ImGui::GetStyle();

	ImVec4* colors = style.Colors;
	colors[ImGuiCol_FrameBg] = ImVec4(0.48f, 0.40f, 0.16f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.98f, 0.77f, 0.26f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.98f, 0.77f, 0.26f, 0.67f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.48f, 0.40f, 0.16f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.98f, 0.77f, 0.26f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.88f, 0.69f, 0.24f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.98f, 0.77f, 0.26f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.98f, 0.77f, 0.26f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.98f, 0.77f, 0.26f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.98f, 0.65f, 0.06f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.98f, 0.77f, 0.26f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.98f, 0.77f, 0.26f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.98f, 0.77f, 0.26f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.75f, 0.54f, 0.10f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.75f, 0.54f, 0.10f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.98f, 0.77f, 0.26f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.98f, 0.77f, 0.26f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.98f, 0.77f, 0.26f, 0.95f);
	colors[ImGuiCol_Tab] = ImVec4(0.58f, 0.47f, 0.18f, 0.86f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.98f, 0.77f, 0.26f, 0.80f);
	colors[ImGuiCol_TabActive] = ImVec4(0.68f, 0.54f, 0.20f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.13f, 0.07f, 0.97f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.42f, 0.34f, 0.14f, 1.00f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.98f, 0.77f, 0.26f, 0.70f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.98f, 0.77f, 0.26f, 0.35f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.98f, 0.77f, 0.26f, 1.00f);

	style.WindowRounding = 2;
	style.ScrollbarRounding = 2;
}

#endif
