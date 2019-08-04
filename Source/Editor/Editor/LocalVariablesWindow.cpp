// Copyright 2015-2019 Piperift - All rights reserved

#include "LocalVariablesWindow.h"
#include "Editor.h"

void LocalVariablesWindow::Build()
{
	Super::Build();

	SetName(TX("Local Variables"));

	Ptr<CodeEditor> codeEditor = GetOwner().Cast<CodeEditor>();
	windowClass = codeEditor->GetCodeDockClass();
	windowFlags |= ImGuiWindowFlags_NoCollapse;
}
