// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#if WITH_EDITOR

#include "../PropertyWidget.h"

#include "Core/Reflection/Runtime/TPropertyHandle.h"


class StringPropertyWidget : public PropertyWidget {
	CLASS(StringPropertyWidget, PropertyWidget)

protected:

	virtual void Tick(float) override;

	TPropertyHandle<String>* GetHandle() const {
		return dynamic_cast<TPropertyHandle<String>*>(prop.get());
	}
};

#endif
