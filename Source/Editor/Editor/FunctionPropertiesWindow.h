// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreObject.h"
#include "UI/Window.h"


class FunctionPropertiesWindow : public Window
{
	CLASS(FunctionPropertiesWindow, Window)

public:

	virtual void Build() override;
	virtual void TickContent(float deltaTime) override;

private:

	void DrawTypeSelector(const char* label, const char* selected);
};

