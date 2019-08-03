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

	TArray<GlobalPtr<CodeEditor>> editors;

	GlobalPtr<AssetBrowser> assetBrowser;
	GlobalPtr<LogWindow> log;

	ImGuiWindowClass mainDockClass;
	ImGuiID mainDock;
	// Dock to which recently opened assets dock
	ImGuiID assetDock;

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
	void TickDocking();

	void DrawMainNavBar();

	template<typename EditorType>
	void CreateEditor()
	{
		GlobalPtr<EditorType> editor = Create<EditorType>(Self());

		editor->Configure();
		editor->OnBuild();

		editors.Add(MoveTemp(editor));
	}

	static void AddFont(Name name, Path path, u8 size = 14u);
	static void PushFont(Name name);
	static void PopFont();

private:

	void ApplyStyle();
};
