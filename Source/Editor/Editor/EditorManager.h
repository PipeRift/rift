// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#if WITH_EDITOR

#include "CoreObject.h"
#include "Editor.h"

#include <imgui/imgui.h>

#include "Core/Containers/Map.h"
#include "Core/Files/FileSystem.h"
#include "UI/Widget.h"
#include "Windows/MemoryDebugger.h"
#include "Windows/AssetBrowser.h"
#include "Scene/SceneEditor.h"
#include "Scene/LogWindow.h"


class EditorManager : public Object {
	CLASS(EditorManager, Object)

	TArray<GlobalPtr<Editor>> editors;

	GlobalPtr<AssetBrowser> assetBrowser;
	GlobalPtr<LogWindow> log;

	Ptr<SceneEditor> sceneEditor;

	bool showDemoWindow = true;

	static TMap<Name, ImFont*> fonts;
	static const String configPath;


public:

	virtual void Construct() override;

	void Tick(float deltaTime);

	void TickDocking();
	void TickMainNavBar();

	void OnSDLEvent(union SDL_Event* ev);

	template<typename EditorType>
	Ptr<EditorType> CreateEditor()
	{
		const i32 i = editors.Add(Create<EditorType>(Self()));
		return editors[i].AsPtr().Cast<EditorType>();
	}

	static void AddFont(Name name, Path path, u8 size = 14u);
	static void PushFont(Name name);
	static void PopFont();

private:

	void ApplyStyle();
};

#endif
