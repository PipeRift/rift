// Copyright 2015-2019 Piperift - All rights reserved

#include "MemberFunctionsWindow.h"
#include "Editor.h"

void MemberFunctionsWindow::Build()
{
	Super::Build();

	SetName(TX("Functions"));

	Ptr<CodeEditor> codeEditor = GetOwner().Cast<CodeEditor>();
	windowClass = codeEditor->GetCodeDockClass();
	windowFlags |= ImGuiWindowFlags_NoCollapse;
}