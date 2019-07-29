// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreObject.h"

#if WITH_EDITOR
#include "UI/Widget.h"
#include "ECS/EntityId.h"
#include "Core/Math/Transform.h"
#include "Core/Input/Keys.h"

enum class EAxisEditMode : u8 {
	Translate,
	Rotate,
	Scale,
	Bounds
};

class Guizmo : public Widget {
	CLASS(Guizmo, Widget)

	EntityId entity = NoEntity;
	EAxisEditMode editMode = EAxisEditMode::Translate;


public:

	virtual void Construct() override;

	virtual void Tick(float deltaTime) override;

	void SetEntity(EntityId inEntity);

	bool IsActive() const;

	void SetTranslateMode(EKeyPressState) { editMode = EAxisEditMode::Translate; }
	void SetRotateMode(EKeyPressState)    { editMode = EAxisEditMode::Rotate;    }
	void SetScaleMode(EKeyPressState)     { editMode = EAxisEditMode::Scale;     }
	void SetBoundsMode(EKeyPressState)    { editMode = EAxisEditMode::Bounds;    }
};

#endif
