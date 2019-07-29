// Copyright 2015-2019 Piperift - All rights reserved

#include "SceneEditor.h"

#if WITH_EDITOR
#include "World.h"


void SceneEditor::Construct()
{
	Super::Construct();

	sceneEntities = Widget::CreateStandalone<SceneEntities>(Self());
	sceneEntities->onSelectionChanged.Bind(this, &SceneEditor::OnSelectionChanged);

	details = Widget::CreateStandalone<SceneDetails>(Self());
	// Select scene
	details->SetObject(GetWorld()->GetActiveScene().Get());

	singletonDetails = Widget::CreateStandalone<SingletonsDetails>(Self());

	guizmo = Widget::CreateStandalone<Guizmo>(Self());
}

void SceneEditor::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	guizmo->Tick(deltaTime);

	sceneEntities->OnTick(deltaTime);
	details->OnTick(deltaTime);
	singletonDetails->OnTick(deltaTime);
}

void SceneEditor::ExpandViewsMenu()
{
	if (ImGui::MenuItem("Entities", (const char*)0, sceneEntities->IsOpenedPtr())) {}
	if (ImGui::MenuItem("Details", (const char*)0, details->IsOpenedPtr())) {}
	if (ImGui::MenuItem("Singletons", (const char*)0, singletonDetails->IsOpenedPtr())) {}
}

void SceneEditor::OnSelectionChanged(const TArray<EntityId>& entities)
{
	selections = entities;
	if (selections.Size() > 0)
	{
		details->SetEntity(selections[0]);
		guizmo->SetEntity(selections[0]);
	}
	else
	{
		details->SetObject(GetWorld()->GetActiveScene().Get());
		guizmo->SetEntity(NoEntity);
	}
}

#endif
