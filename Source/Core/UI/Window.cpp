// Copyright 2015-2019 Piperift - All rights reserved

#include "Window.h"


void Window::Build()
{
	bOpen = true;
	bWindowOpened = true;
	windowFlags = 0;
}

void Window::Tick(float deltaTime)
{
	if (bOpen)
	{
		BeginWindow();
		if (bWindowOpened)
		{
			TickContent(deltaTime);
		}
		EndWindow();
	}
}

void Window::BeginWindow()
{
	// Generate unique name
	const String finalName = GetWindowID();

	ImGui::SetNextWindowClass(&windowClass);
	bWindowOpened = ImGui::Begin(finalName.c_str(), &bOpen, windowFlags);
}

void Window::EndWindow()
{
	ImGui::End();
}

const String Window::GetWindowID() const
{
	if (!displayName.empty())
	{
		return CString::Printf("%s###%s", displayName.c_str(), GetName().ToString().c_str());
	}
	return GetName().ToString();

}
