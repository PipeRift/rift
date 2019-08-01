// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#if WITH_EDITOR

#include "UI/Window.h"


class MemoryDebugger : public Window {
	CLASS(MemoryDebugger, Window)

protected:

	virtual void Build() override;

	virtual void Tick(float deltaTime) override;
};

#endif
