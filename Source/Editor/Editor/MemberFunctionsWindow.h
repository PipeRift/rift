// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreObject.h"
#include "UI/Window.h"


class MemberFunctionsWindow : public Window
{
	CLASS(MemberFunctionsWindow, Window)

	Name selectedFunction;

public:

	virtual void Build() override;
	virtual void TickContent(float deltaTime) override;
};
