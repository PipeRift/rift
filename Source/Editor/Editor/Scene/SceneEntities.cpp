// Copyright 2015-2019 Piperift - All rights reserved

#include "SceneEntities.h"
#include "Core/Log.h"
#include "Core/Serialization/Archive.h"
#include "World.h"
#include "ECS/ECSManager.h"
#include "Gameplay/Components/CEntity.h"
#include "Core/Engine.h"


#if WITH_EDITOR

void SceneEntities::Build()
{
	Super::Build();
	SetName(TX("Entities"));
	windowFlags = ImGuiWindowFlags_AlwaysAutoResize;
}

void SceneEntities::Tick(float)
{
	Ptr<World> world = GetWorld();

	if (bOpen && world)
	{
		auto ecsManager = world->GetECS();

		if (ecsManager)
		{
			BeginWindow();

			if (ImGui::Button("New Entity"))
			{
				EntityId entity = ecsManager->CreateEntity("");
				OnEntityClicked(entity);
			}

			filter.Draw("##FilterDraw");

			auto view = ecsManager->View<CEntity>();
			i32 numOfEntities = (i32)view.size();

			if (ImGui::ListBoxHeader("##EntityContainer", numOfEntities, 10))
			{
				ImGuiListClipper clipper(numOfEntities, ImGui::GetTextLineHeightWithSpacing());
				while (clipper.Step())
				{
					for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
					{
						EntityId entity = *(view.begin() + i);
						String entName = view.get(entity).name.ToString();

						ImGui::PushID(i);
						if (filter.PassFilter(entName.begin(), entName.end()))
						{
							const bool selected = selectedEntities.Contains(entity);
							if (ImGui::Selectable(entName.c_str(), selected) ||
								ImGui::OpenPopupOnItemClick("EntityContextMenu", 1))
							{
								OnEntityClicked(entity);
							}
							DrawEntityContextMenu(entity);
						}
						ImGui::PopID();
					}
				}
				ImGui::ListBoxFooter();
			}
			EndWindow();

			// Delete entities marked for deletion after loop has finished
			for (EntityId e : deletedEntities)
			{
				ecsManager->DestroyEntity(e);
			}
			deletedEntities.Empty();
		}
	}
}

void SceneEntities::OnEntityClicked(EntityId entity)
{
	auto input = GEngine->GetInput();

	const i32 index = selectedEntities.FindIndex(entity);
	if (index == NO_INDEX)
	{
		// If Ctrl is pressed, add the entity to the list
		if (!input->IsModifierPressed(EKeyModifier::Ctrl))
		{
			selectedEntities.Empty();
		}
		selectedEntities.Add(entity);
	}
	else
	{
		// Already selected select
		if (!input->IsModifierPressed(EKeyModifier::Ctrl))
		{
			selectedEntities.Empty();
		}
		else
		{
			selectedEntities.RemoveAt(index);
		}
	}

	onSelectionChanged.DoBroadcast(selectedEntities);
}

void SceneEntities::DrawEntityContextMenu(EntityId entity)
{
	if (ImGui::BeginPopupContextItem("EntityContextMenu"))
	{
		if (ImGui::Selectable("Copy")) {}
		if (ImGui::Selectable("Paste")) {}
		if (ImGui::Selectable("Delete"))
		{
			deletedEntities.Add(entity);
		}
		ImGui::EndPopup();
	}
}

#endif
