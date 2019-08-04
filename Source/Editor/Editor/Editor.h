// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreObject.h"

#include "UI/Window.h"

#include "Assets/BaseAsset.h"
#include "MemberVariablesWindow.h"
#include "MemberFunctionsWindow.h"
#include "LocalVariablesWindow.h"
#include "FunctionGraphWindow.h"
#include "Core/Object/ObjectPtr.h"


class Editor : public Window
{
	CLASS(Editor, Window)
};

class NewPageEditor : public Editor
{
	CLASS(NewPageEditor, Editor)

public:

	virtual void Build() override;
};

class CodeEditor : public Editor
{
	CLASS(CodeEditor, Editor)

	static u32 codeEditorCount;

	Ptr<MemberVariablesWindow> variables;
	Ptr<MemberFunctionsWindow> functions;
	Ptr<LocalVariablesWindow> localVariables;
	Ptr<FunctionGraphWindow> functionGraph;

	Ptr<BaseAsset> asset;

	ImGuiID codeDock;
	ImGuiWindowClass codeDockClass;

public:

	virtual void Build() override;

	virtual void Tick(float deltaTime) override;

	void ApplyLayoutPreset();

	virtual void ExpandViewsMenu() {}

	const ImGuiWindowClass& GetCodeDockClass() { return codeDockClass; }
};

