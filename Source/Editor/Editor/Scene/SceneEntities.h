// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#if WITH_EDITOR

#include "Editor/EditorWindow.h"
#include "Editor/Widgets/PropertyWidget.h"

#include "ECS/ECSManager.h"


class SceneEntities : public EditorWindow {
	CLASS(SceneEntities, EditorWindow)

	ImGuiTextFilter filter;

	TArray<EntityId> selectedEntities;
	TArray<EntityId> deletedEntities;

public:

	Broadcast<const TArray<EntityId>&> onSelectionChanged;


	SceneEntities() : Super(), filter{} {}

protected:

	virtual void Build() override;

	virtual void Tick(float deltaTime) override;

	void OnEntityClicked(EntityId entity);

	void DrawEntityContextMenu(EntityId entity);
};

#endif
