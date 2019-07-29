// Copyright 2015-2019 Piperift - All rights reserved

#include "Transform.h"
#include "Math.h"
#include "Core/Serialization/Archive.h"

#if WITH_EDITOR
#include <EASTL/shared_ptr.h>
#include <imgui/imgui.h>

#include "Core/Reflection/Class.h"
#include "Core/Reflection/Runtime/TPropertyHandle.h"
#include "Editor/Widgets/PropertyWidget.h"
#endif


bool Transform::Serialize(Archive& ar, const char* name)
{
	ar.BeginObject(name);
	{
		ar("location", location);
		ar("rotation", rotation);
		ar("scale", scale);
	}
	ar.EndObject();
	return true;
}


#if WITH_EDITOR

class TransformPropertyWidget : public PropertyWidget {
	CLASS(TransformPropertyWidget, PropertyWidget)

protected:

	virtual void Tick(float) override {

		ImGuiInputTextFlags flags = 0;
		if (!prop->HasTag(DetailsEdit))
			flags |= ImGuiInputTextFlags_ReadOnly;

		ImGui::PushID(GetName().ToString().c_str());
		{
			ImGui::Text(displayName.c_str());

			Transform& tr = *GetHandle()->GetValuePtr();

			ImGui::InputFloat3("Location", tr.location.Data(), 3, flags);

			v3 rotation = tr.GetRotation();
			v3 prevRotation = rotation;
			ImGui::InputFloat3("Rotation", rotation.Data(), 3);
			if (rotation != prevRotation)
			{
				tr.SetRotation(rotation);
			}

			ImGui::InputFloat3("Scale", tr.scale.Data(), 3, flags);
		}
		ImGui::PopID();
	}

	TPropertyHandle<Transform>* GetHandle() const {
		return dynamic_cast<TPropertyHandle<Transform>*>(prop.get());
	}
};

Class* Transform::GetDetailsWidgetClass()
{
	return TransformPropertyWidget::StaticClass();
}

#endif
