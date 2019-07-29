// Copyright 2015-2019 Piperift - All rights reserved

#include "SingletonsDetails.h"

#include "World.h"
#include "ECS/ECSManager.h"

#include "Widgets/SingletonDetails.h"

#include "Gameplay/Singletons/CPhysicsWorld.h"
#include "Gameplay/Singletons/CSceneTree.h"
#include "Gameplay/Singletons/CVisualDebugger.h"
#include "Gameplay/Singletons/CGraphics.h"


#if WITH_EDITOR

void SingletonsDetails::Construct()
{
	World::Delegates::onBeginPlay.Bind(this, &SingletonsDetails::OnChangePlay);
	World::Delegates::onEndPlay.Bind(this, &SingletonsDetails::OnChangePlay);
}

void SingletonsDetails::Build()
{
	Super::Build();
	SetName(TX("Singletons"));

	entries.Empty();

	auto ecs = GetWorld()->GetECS();

	AddNew<SingletonDetails<CPhysicsWorld>>();
	AddNew<SingletonDetails<CGraphics>>();
	AddNew<SingletonDetails<CSceneTree>>();
	AddNew<SingletonDetails<CVisualDebugger>>();
}

void SingletonsDetails::Tick(float deltaTime)
{
	if (bOpen)
	{
		BeginWindow();

		TickChilds(deltaTime);

		ImGui::Spacing();
		ImGui::Separator();
		if (ImGui::BeginCombo("##AddSingleton", "Add Singleton", ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_HeightLarge))
		{
			// Focus Filter by default
			if (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
				ImGui::SetKeyboardFocusHere(0);

			static ImGuiTextFilter filter;
			filter.Draw("##Search", ImGui::GetContentRegionAvail().x - 5);

			ImGui::Spacing();

			ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2{});
			for (const auto& entry : entries)
			{
				if (filter.PassFilter(entry.displayName.c_str(), entry.displayName.end()) &&
					ImGui::Button(entry.displayName.c_str(), ImVec2(ImGui::GetContentRegionAvail().x - 5, 0.0f)))
				{
					onSingletonAdded.DoBroadcast(entry);
					ForceRebuild();

					if (entries.Size() <= 0)
					{
						// No more components, close
						ImGui::CloseCurrentPopup();
					}
					break;
				}
			}
			ImGui::PopStyleVar(1);
			ImGui::EndCombo();
		}

		EndWindow();
	}
}

void SingletonsDetails::AddComponentEntry(ComponentCreationEntry&& entry)
{
	entries.Add(MoveTemp(entry));
}

void SingletonsDetails::OnChangePlay(Ptr<World> world)
{
	if (world == GetWorld())
	{
		ForceRebuild();
	}
}

#endif
