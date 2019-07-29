// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreObject.h"

#include "SceneEntities.h"
#include "SceneDetails.h"
#include "SingletonsDetails.h"
#include "Guizmo.h"
#include "Editor/Editor.h"


#if WITH_EDITOR

class SceneEditor : public Editor {
	CLASS(SceneEditor, Editor)

	GlobalPtr<SceneEntities> sceneEntities;
	GlobalPtr<SceneDetails> details;
	GlobalPtr<SingletonsDetails> singletonDetails;

	GlobalPtr<Guizmo> guizmo;

public:

	TArray<EntityId> selections;


	virtual void Construct() override;

	virtual void Tick(float deltaTime) override;

	virtual void ExpandViewsMenu() override;

	void OnSelectionChanged(const TArray<EntityId>& entities);
};

#endif
