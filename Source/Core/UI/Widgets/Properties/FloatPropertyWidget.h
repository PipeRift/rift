// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#if WITH_EDITOR

#include "../PropertyWidget.h"

#include <imgui/imgui.h>
#include "Core/Reflection/Runtime/TPropertyHandle.h"


class FloatPropertyWidget : public PropertyWidget {
	CLASS(FloatPropertyWidget, PropertyWidget)

protected:

	virtual void Tick(float) override;

	TPropertyHandle<float>* GetHandle() const {
		return dynamic_cast<TPropertyHandle<float>*>(prop.get());
	}
};

#endif
