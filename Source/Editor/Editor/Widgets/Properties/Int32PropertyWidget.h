// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#if WITH_EDITOR

#include "../PropertyWidget.h"

#include <imgui/imgui.h>
#include "Core/Reflection/Runtime/TPropertyHandle.h"


class Int32PropertyWidget : public PropertyWidget {
	CLASS(Int32PropertyWidget, PropertyWidget)

protected:

	virtual void Tick(float) override;

	TPropertyHandle<i32>* GetHandle() const {
		return dynamic_cast<TPropertyHandle<i32>*>(prop.get());
	}
};

#endif
