// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "UI/Style.h"
#include "UI/UIImgui.h"
#include "UI/Widgets.h"

#include <Misc/PipeImGui.h>
#include <PipeReflect.h>


namespace rift::UI
{
	using CustomKeyValue = p::TFunction<void(p::StringView label, void* data, p::TypeId type)>;

	// Registers additional type inspections when an inspector appears
	using RegisterInspections = p::TFunction<void()>;


	// Registers a custom single-row inspection for a type. The callback draws both the key
	// and the value.
	void RegisterCustomInspection(p::TypeId type, const CustomKeyValue& custom);
	template<typename T>
	void RegisterCustomInspection(const CustomKeyValue& custom)
	{
		RegisterCustomInspection(p::GetTypeId<T>(), custom);
	}

	// Inspects the properties of a reflected struct. Only properties marked as editable are
	// shown when the inspector was opened with BeginInspector(editableOnly = true).
	void InspectStruct(void* data, p::TypeId type);
	template<typename T>
	void InspectStruct(T* data) requires(p::IsStructOrClass<T>)
	{
		InspectStruct(data, p::GetTypeId<T>());
	}

	void InspectChildrenProperties(void* data, p::TypeId type);

	bool BeginCategory(p::StringView name, bool isLeaf);
	void EndCategory();

	// Opens a reflection inspector. When editableOnly is true, properties without PF_Edit are
	// hidden instead of being disabled. registerTypes is invoked on the appearing frame, so
	// callers can register their own type inspections into the current context. idContext is an
	// optional Id registry used by Id related inspections.
	bool BeginInspector(const char* name, p::v2 size = p::v2{0.f, 0.f}, bool editableOnly = false,
	    RegisterInspections registerTypes = {}, p::IdContext* idContext = nullptr);
	void EndInspector();
}    // namespace rift::UI
