// Copyright 2015-2019 Piperift - All rights reserved

#include "FunctionGraphWindow.h"
#include "Editor.h"

void FunctionGraphWindow::Build()
{
	Super::Build();

	SetName(TX("Function Graph"));

	Ptr<CodeEditor> codeEditor = GetOwner().Cast<CodeEditor>();
	windowClass = codeEditor->GetCodeDockClass();
	windowFlags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
}
