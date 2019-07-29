// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#if WITH_EDITOR

#include "../PropertyWidget.h"

#include <imgui/imgui.h>
#include "Core/Reflection/Runtime/TPropertyHandle.h"


class V2PropertyWidget : public PropertyWidget {
	CLASS(V2PropertyWidget, PropertyWidget)

protected:

	virtual void Tick(float) override;

	TPropertyHandle<v2>* GetHandle() const {
		return dynamic_cast<TPropertyHandle<v2>*>(prop.get());
	}
};

#endif
