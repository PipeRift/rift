// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreObject.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>


#define W(Widget, ...) AddNew<Widget>(__VA_ARGS__)

class Widget : public Object {
	CLASS(Widget, Object)

	TArray<GlobalPtr<Widget>> children;

	bool bBuilding = false;
	bool bBuilt = false;

protected:

	bool bDisabled = false;


public:

	Widget() : Super() {}

	/** Called after widget creation with parameters.
	 * For custom parameters create your own
	 * void Configure(...)
	 */
	virtual void Configure() {}

	/** called AFTER parent widget is built */
	void OnBuild() {
		if (!bBuilt && !bBuilding)
		{
			bBuilding = true;
			Build();
			bBuilding = false;
			bBuilt = true;

			for (auto& child : children)
			{
				child->OnBuild();
			}
		}
	}

	void ForceRebuild() {
		if (!bBuilding)
		{
			bBuilt = false;
			UndoBuild();
			children.Empty(false);
			OnBuild();
		}
	}

	void OnTick(float deltaTime) {
		if (bBuilt)
		{
			const bool bDisabledNow = bDisabled;
			if (bDisabledNow)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}
			Tick(deltaTime);
			if (bDisabledNow)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
	}

	void TickChilds(float deltaTime) {
		for (auto& child : children)
		{
			child->OnTick(deltaTime);
		}
	}

protected:
	/** Create child widgets here with W() */
	virtual void Build() {}
	/** Called before rebuilding or when the widget gets destroyed */
	virtual void UndoBuild() {}

	virtual void Tick(float deltaTime) { TickChilds(deltaTime); }

	virtual void BeforeDestroy() override {
		UndoBuild();
		Super::BeforeDestroy();
	}


public:

	/** Create widget from template type */
	template<typename WidgetType, typename... Args>
	GlobalPtr<WidgetType> New(Args&&... args) {
		// Create the widget
		GlobalPtr<WidgetType> widget = Create<WidgetType>(Self());

		// Configure it
		widget->Configure(std::forward<Args>(args)...);

		return widget;
	}

	/** Create widget from class */
	template<typename WidgetType, typename... Args>
	GlobalPtr<WidgetType> New(Class* c, Args&&... args) {
		// Create the widget
		GlobalPtr<WidgetType> widget = Create<WidgetType>(c, Self());

		widget->Configure(std::forward<Args>(args)...);

		return widget;
	}

	Ptr<Widget> Add(GlobalPtr<Widget>&& widget) {
		if (bBuilding && widget)
		{
			// Registry the widget
			Ptr<Widget> ptr = widget;
			children.Add(std::move(widget));

			return ptr;
		}
		return {};
	}

	template<typename WidgetType, typename... Args>
	Ptr<WidgetType> AddNew(Args&&... args) {
		return Add(New<WidgetType>(std::forward<Args>(args)...)).Cast<WidgetType>();
	}

	template<typename WidgetType, typename... Args>
	static GlobalPtr<WidgetType> CreateStandalone(Ptr<Object> owner, Args&&... args) {
		// Create the widget
		GlobalPtr<WidgetType> widget = Create<WidgetType>(owner);

		// Configure it
		widget->Configure(std::forward<Args>(args)...);
		widget->OnBuild();
		return std::move(widget);
	}
};
