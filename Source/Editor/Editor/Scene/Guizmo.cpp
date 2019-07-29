// Copyright 2015-2019 Piperift - All rights reserved

#include "Guizmo.h"

#if WITH_EDITOR
#include <ImGui/imgui.h>
#include <ImGui/ImGuizmo.h>

#include "World.h"
#include "Core/Reflection/Runtime/HandleHelper.h"
#include "Core/Reflection/ReflectionTags.h"
#include "Core/Object/Object.h"
#include "ECS/ECSManager.h"
#include "ECS/Component.h"

#include "Gameplay/Components/CTransform.h"
#include "Gameplay/Singletons/CActiveCamera.h"
#include "Core/Engine.h"


void Guizmo::Construct()
{
	// Bind events
	auto input = GEngine->GetInput();
	input->CreateTriggerAction("GizmoModeTranslate", {
		EKey::E
	}).Bind(this, &Guizmo::SetTranslateMode);

	input->CreateTriggerAction("GizmoModeRotate", {
		EKey::R
	}).Bind(this, &Guizmo::SetRotateMode);

	input->CreateTriggerAction("GizmoModeScale", {
		EKey::T
	}).Bind(this, &Guizmo::SetScaleMode);
}

void Guizmo::Tick(float deltaTime)
{
	if (!IsActive())
	{
		return;
	}

	auto ecs = GetWorld()->GetECS();
	assert(ecs.IsValid());

	if (auto* cameraComp = ecs->FindSingleton<CActiveCamera>())
	{
		CTransform& transform = ecs->Get<CTransform>(entity);

		ImGuiViewport* vp = ImGui::GetMainViewport();
		Matrix4f projection = cameraComp->activeData.GetProjectionMatrix({ vp->Size.x, vp->Size.y });
		Matrix4f view       = cameraComp->activeData.GetViewMatrix();

		ImGuizmo::SetRect(vp->Pos.x, vp->Pos.y, vp->Size.x, vp->Size.y);

		// Draw Grid
		/*{
			Transform gridTransform {};
			gridTransform.location = glm::step(1.f, transform.transform.location);
			gridTransform.location.z = 0.f;
			gridTransform.SetRotation({-90.f, 0.f, 0.f});
			ImGuizmo::DrawGrid(view.Data(), projection.Data(), gridTransform.ToMatrix().Data(), 10.f);
		}*/

		// Draw Axis
		{
			Matrix4f matrix = transform.transform.ToMatrix();

			//ImGuizmo::DrawCube(view.Data(), projection.Data(), matrix.Data());

			ImGuizmo::Manipulate(view.Data(), projection.Data(), static_cast<ImGuizmo::OPERATION>(editMode), ImGuizmo::LOCAL, matrix.Data(), nullptr, nullptr);
			if (ImGuizmo::IsUsing())
			{
				transform.transform.SetFromMatrix(matrix);
			}
		}
	}
}

void Guizmo::SetEntity(EntityId inEntity)
{
	if (inEntity != entity)
	{
		entity = inEntity;
		ForceRebuild();
	}
}

bool Guizmo::IsActive() const
{
	auto ecs = GetWorld()->GetECS();
	return ecs->IsValid(entity) && ecs->Has<CTransform>(entity);
}

#endif
