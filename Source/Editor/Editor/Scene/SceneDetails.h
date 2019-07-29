// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#if WITH_EDITOR

#include "Editor/EditorWindow.h"
#include "ECS/EntityId.h"
#include "Core/Events/Broadcast.h"


struct ComponentCreationEntry {
	String displayName;
	class StructType* type;
};


class SceneDetails : public EditorWindow {
	CLASS(SceneDetails, EditorWindow)

	EntityId entity = NoEntity;
	Ptr<Object> object;

	TArray<ComponentCreationEntry> entries;
	Broadcast<const ComponentCreationEntry&> onComponentAdded;


protected:

	virtual void Build() override;

	virtual void Tick(float deltaTime) override;

public:

	void SetEntity(EntityId inEntity);
	void SetObject(Ptr<Object> inObject);

	void AddComponentEntry(ComponentCreationEntry&& entry);

	Broadcast<const ComponentCreationEntry&>& OnComponentAdded() {
		return onComponentAdded;
	}
};

#endif
