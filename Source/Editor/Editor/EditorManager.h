// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreObject.h"
#include "Editor.h"

#include <imgui/imgui.h>

#include "Core/Containers/Map.h"
#include "Core/Files/FileSystem.h"
#include "UI/Widget.h"
#include "UI/Windows/MemoryDebugger.h"
#include "UI/Windows/AssetBrowser.h"
#include "UI/Windows/LogWindow.h"


class EditorManager : public Object {
	CLASS(EditorManager, Object)

	TArray<GlobalPtr<Editor>> editors;

	GlobalPtr<AssetBrowser> assetBrowser;
	GlobalPtr<LogWindow> log;

	Ptr<NewPageEditor> newPage;

	ImGuiWindowClass mainDockClass;
	ImGuiID mainDock = 0;
	// Dock to which recently opened assets dock
	ImGuiID assetDock = 0;

	bool showDemoWindow = true;

	static TMap<Name, ImFont*> fonts;
	static const String configPath;


public:

	EditorManager() : Super()
	{
		mainDockClass.ClassId = 1;
	}

	virtual void Construct() override;

	void Tick(float deltaTime);

	void ApplyLayoutPreset();
	void RestoreLayoutPreset();
	void TickDocking();

	void DrawMainNavBar();

	template<typename EditorType>
	Ptr<EditorType> CreateEditor()
	{
		GlobalPtr<EditorType> editor = Create<EditorType>(Self());

		editor->Configure();
		editor->OnBuild();

		if (assetDock > 0)
		{
			ImGuiUtil::DockBuilderDockWindow(editor, assetDock);
		}

		editors.Add(MoveTemp(editor));
		return editors.Last().AsPtr().Cast<EditorType>();
	}

	static void AddFont(Name name, Path path, u8 size = 14u);
	static void PushFont(Name name);
	static void PopFont();

private:

	void ApplyStyle();
};
