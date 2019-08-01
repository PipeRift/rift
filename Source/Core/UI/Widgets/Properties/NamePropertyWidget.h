// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#if WITH_EDITOR

#include "../PropertyWidget.h"

#include "Core/Reflection/Runtime/TPropertyHandle.h"


class NamePropertyWidget : public PropertyWidget {
	CLASS(NamePropertyWidget, PropertyWidget)

	String currentValue;
	String lastValue;

public:

	NamePropertyWidget() : Super(), lastValue{}, currentValue{} {}

	void OnValueChanged();

protected:

	virtual void Tick(float) override;

	TPropertyHandle<Name>* GetHandle() const {
		return dynamic_cast<TPropertyHandle<Name>*>(prop.get());
	}
};

#endif
