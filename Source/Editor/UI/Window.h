// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "Widget.h"
#include <imgui/imgui.h>


class Window : public Widget {
	CLASS(Window, Widget)

protected:

	bool bOpen;
	String displayName;
	ImGuiWindowFlags windowFlags;


	virtual void Build() override;
	virtual void Tick(float deltaTime) override;

	void BeginWindow();
	void EndWindow();

public:

	void SetOpen(bool bValue) { bOpen = bValue; }
	bool IsOpened() const { return bOpen; }
	bool* IsOpenedPtr() { return &bOpen; }

	const String& GetDisplayName() const {
		return displayName.empty()? GetName().ToString() : displayName;
	}
};