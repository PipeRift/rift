// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "UI/Inspection.h"


namespace rift::UI
{
	namespace
	{
		p::InspectionContext gInspectorCtx;
	}    // namespace

	void RegisterCustomInspection(p::TypeId type, const CustomKeyValue& custom)
	{
		if (!custom)
		{
			return;
		}

		p::RegisterTypeInspection(type,
		    [custom](p::StringView label, void* data, p::TypeId typeId, bool& open)
		{
			open = false;    // Custom inspections always draw a single row
			custom(label, data, typeId);
		});
	}

	void InspectStruct(void* data, p::TypeId type)
	{
		if (p::HasTypeFlags(type, p::TF_Struct))
		{
			p::InspectProperties(data, type);
		}
	}

	void InspectChildrenProperties(void* data, p::TypeId type)
	{
		p::InspectProperties(data, type);
	}

	bool BeginCategory(p::StringView name, bool isLeaf)
	{
		UI::PushHeaderColor(UI::GetNeutralColor(1));
		const bool open = p::InspectBeginCategory(name, isLeaf);
		UI::PopHeaderColor();
		return open;
	}
	void EndCategory()
	{
		p::InspectEndCategory();
	}

	bool BeginInspector(const char* name, p::v2 size, bool editableOnly,
	    RegisterInspections registerTypes, p::IdContext* idContext)
	{
		if (!p::BeginInspection(gInspectorCtx, name, size,
		        editableOnly ? p::IF_None : p::IF_ViewAndEditAll, nullptr, idContext))
		{
			return false;
		}

		// Type inspections belong to the current inspection context, so they are registered
		// when the inspector appears
		if (UI::IsWindowAppearing())
		{
			if (registerTypes)
			{
				registerTypes();
			}
		}
		return true;
	}
	void EndInspector()
	{
		p::EndInspection();
	}
}    // namespace rift::UI
