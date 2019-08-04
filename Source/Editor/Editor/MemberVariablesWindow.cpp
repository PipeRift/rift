// Copyright 2015-2019 Piperift - All rights reserved

#include "MemberVariablesWindow.h"
#include "Editor.h"

void MemberVariablesWindow::Build()
{
	Super::Build();

	SetName(TX("Variables"));

	Ptr<CodeEditor> codeEditor = GetOwner().Cast<CodeEditor>();
	windowClass = codeEditor->GetCodeDockClass();
	windowFlags |= ImGuiWindowFlags_NoCollapse;
}