// Copyright 2015-2019 Piperift - All rights reserved

#include "Window.h"


void Window::Build()
{
	bOpen = true;
	windowFlags = 0;
}

void Window::Tick(float deltaTime)
{
	if (bOpen)
	{
		BeginWindow();
		TickChilds(deltaTime);
		EndWindow();
	}
}

void Window::BeginWindow()
{
	// Generate unique name
	String finalName;
	if (!displayName.empty())
		finalName = CString::Printf("%s###%s", displayName.c_str(), GetName().ToString().c_str());
	else
		finalName = GetName().ToString();

	ImGui::Begin(finalName.c_str(), &bOpen, windowFlags);
}

void Window::EndWindow()
{
	ImGui::End();
}
