// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#if WITH_EDITOR

#include "UI/Widget.h"

#include <imgui/imgui.h>

#include "Core/Strings/String.h"
#include "Core/Reflection/Runtime/PropertyHandle.h"


class PropertyWidget : public Widget {
	CLASS(PropertyWidget, Widget)

protected:

	String displayName;

	eastl::shared_ptr<PropertyHandle> prop;

	virtual void Build() override {
		Super::Build();
		bDisabled = prop->HasTag(DetailsView) && !prop->HasTag(DetailsEdit);
	}

	virtual void Tick(float) override {}

public:

	void Configure(const eastl::shared_ptr<PropertyHandle>& inProperty)
	{
		prop = inProperty;
		SetName(prop->GetName());
		CString::ToSentenceCase(GetName().ToString(), displayName);
	}

	static GlobalPtr<PropertyWidget> NewPropertyWidget(const Ptr<Widget>& owner, const eastl::shared_ptr<PropertyHandle>& prop);
};

#endif
