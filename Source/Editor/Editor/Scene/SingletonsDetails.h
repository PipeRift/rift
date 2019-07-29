// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#if WITH_EDITOR

#include "Editor/EditorWindow.h"
#include "Core/Events/Broadcast.h"
#include "SceneDetails.h"


class SingletonsDetails : public EditorWindow {
	CLASS(SingletonsDetails, EditorWindow)

	TArray<ComponentCreationEntry> entries;
	Broadcast<const ComponentCreationEntry&> onSingletonAdded;


public:

	virtual void Construct() override;

protected:

	virtual void Build() override;

	virtual void Tick(float deltaTime) override;

public:

	void AddComponentEntry(ComponentCreationEntry&& entry);

	Broadcast<const ComponentCreationEntry&>& OnSingletonAdded() {
		return onSingletonAdded;
	}

private:

	void OnChangePlay(class Ptr<World> world);
};

#endif
