// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#if WITH_EDITOR

#include "UI/Widget.h"
#include "ECS/ECSManager.h"
#include "Gameplay/Components/CEntity.h"
#include "../SceneDetails.h"


template<typename Type>
class ComponentDetails : public Widget {
	CLASS(ComponentDetails<Type>, Widget)
public:

	Ptr<ECSManager> ecs;
	EntityId entity = NoEntity;

	String displayName;


	void Configure(EntityId inEntity)
	{
		entity = inEntity;
		ecs = GetWorld()->GetECS();
	}

protected:

	virtual void Build() override;
	virtual void Tick(float) override;

private:

	void OnComponentAdded(const ComponentCreationEntry& entry)
	{
		if (entry.type == Type::StaticStruct())
		{
			ecs->Assign<Type>(entity);
		}
	}
};

template<typename Type>
void ComponentDetails<Type>::Build()
{
	if (!ecs->IsValid(entity))
	{
		return;
	}

	StructType* type = Type::StaticStruct();
	displayName = type->GetSName();
	CString::RemoveFromStart(displayName, 1); // Remove C prefix
	CString::ToSentenceCase(displayName, displayName);

	if (ecs->Has<Type>(entity))
	{
		Type& comp = ecs->Get<Type>(entity);

		// Display each component property
		PropertyMap properties{ Name::None() };
		Type::StaticStruct()->GetAllProperties(properties);
		for (const auto& property : properties)
		{
			String str = property.first.ToString();
			if (property.second->HasTag(DetailsEdit) || property.second->HasTag(DetailsView))
			{
				auto handle = property.second->CreateHandle(&comp);
				Add(PropertyWidget::NewPropertyWidget(Self<Widget>(), handle));
			}
		}
	}
	else
	{
		Ptr<SceneDetails> details = GetOwner().Cast<SceneDetails>();
		details->AddComponentEntry({
			displayName,
			type
		});

		details->OnComponentAdded().Bind(this, &ComponentDetails<Type>::OnComponentAdded);
	}
}

template<typename Type>
void ComponentDetails<Type>::Tick(float deltaTime)
{
	if (ecs->IsValid(entity) && ecs->Has<Type>(entity))
	{
		bool bNotCollapsed = ImGui::CollapsingHeader(displayName.c_str(), ImGuiTreeNodeFlags_AllowOverlapMode);

		if(!eastl::is_convertible<Type, CEntity>::value)
		{
			ImGui::SameLine(ImGui::GetWindowWidth() - 30);

			//Remove button. Won't appear on CEntity
			String label = CString::Printf("X##%i", Type::StaticStruct());
			if (ImGui::Button(label.c_str()))
			{
				ecs->Remove<Type>(entity);

				// Rebuild full details panel
				GetOwner().Cast<SceneDetails>()->ForceRebuild();
				bNotCollapsed = false;
			}
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
