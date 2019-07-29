// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#if WITH_EDITOR

#include "UI/Widget.h"
#include "ECS/ECSManager.h"
#include "Editor/Widgets/PropertyWidget.h"
#include "../SingletonsDetails.h"


template<typename Type>
class SingletonDetails : public Widget {
	CLASS(SingletonDetails<Type>, Widget)
public:

	Ptr<ECSManager> ecs;

	String displayName;


	virtual void Configure() override
	{
		ecs = GetWorld()->GetECS();
	}

protected:

	virtual void Build() override;
	virtual void Tick(float) override;

private:

	void OnSingletonAdded(const ComponentCreationEntry& entry)
	{
		if (entry.type == Type::StaticStruct())
		{
			ecs->AssignSingleton<Type>();
		}
	}
};

template<typename Type>
void SingletonDetails<Type>::Build()
{
	StructType* type = Type::StaticStruct();
	displayName = type->GetSName();
	CString::RemoveFromStart(displayName, 1); // Remove C prefix
	CString::ToSentenceCase(displayName, displayName);

	if (Type* comp = ecs->FindSingleton<Type>())
	{
		// Display each component property
		PropertyMap properties{ Name::None() };
		Type::StaticStruct()->GetAllProperties(properties);
		for (const auto& property : properties)
		{
			if (property.second->HasTag(DetailsEdit) || property.second->HasTag(DetailsView))
			{
				auto handle = property.second->CreateHandle(comp);
				Add(PropertyWidget::NewPropertyWidget(Self<Widget>(), handle));
			}
		}
	}
	else
	{
		Ptr<SingletonsDetails> details = GetOwner().Cast<SingletonsDetails>();
		details->AddComponentEntry({
			displayName,
			type
		});

		details->OnSingletonAdded().Bind(this, &SingletonDetails<Type>::OnSingletonAdded);
	}
}

template<typename Type>
void SingletonDetails<Type>::Tick(float deltaTime)
{
	if (ecs->HasSingleton<Type>())
	{
		bool bNotCollapsed = ImGui::CollapsingHeader(displayName.c_str(), ImGuiTreeNodeFlags_AllowOverlapMode);

		ImGui::SameLine(ImGui::GetWindowWidth() - 30);

		//Remove button. Won't appear on CEntity
		String label = CString::Printf("X##%i", Type::StaticStruct());
		if (ImGui::Button(label.c_str()))
		{
			ecs->RemoveSingleton<Type>();

			// Rebuild full details panel
			GetOwner().Cast<SingletonsDetails>()->ForceRebuild();
			bNotCollapsed = false;
		}

		// Show properties
		if(bNotCollapsed)
		{
			ImGui::Indent();

			TickChilds(deltaTime);

			ImGui::Spacing();
			ImGui::Unindent();
		}
	}
}

#endif
