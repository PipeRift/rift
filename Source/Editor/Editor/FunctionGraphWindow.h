// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreObject.h"
#include "UI/Window.h"


class FunctionGraphWindow : public Window
{
	CLASS(FunctionGraphWindow, Window)

public:

	virtual void Build() override;
	virtual void Tick(float deltaTime) override;
};
