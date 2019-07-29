// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#if WITH_EDITOR

#include "../PropertyWidget.h"

#include <imgui/imgui.h>
#include "Core/Reflection/Runtime/TPropertyHandle.h"


class BoolPropertyWidget : public PropertyWidget {
	CLASS(BoolPropertyWidget, PropertyWidget)

protected:

	virtual void Tick(float) override;

	TPropertyHandle<bool>* GetHandle() const {
		return dynamic_cast<TPropertyHandle<bool>*>(prop.get());
	}
};

#endif
