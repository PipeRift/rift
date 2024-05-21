// Copyright 2015-2024 Piperift - All rights reserved

#include "Tools/ASTDebugger.h"

#include "imgui.h"
#include "PipeReflect.h"
#include "UI/Widgets.h"

#include <AST/Components/Statements.h>
#include <AST/Statics/STypes.h>
#include <AST/Tree.h>
#include <AST/Utils/Namespaces.h>
#include <AST/Utils/Paths.h>
#include <IconsFontAwesome5.h>
#include <Pipe/Core/PlatformMisc.h>
#include <UI/Inspection.h>
#include <UI/UI.h>


#define P_DEBUG_IMPLEMENTATION
#include <Misc/PipeDebug.h>


namespace rift::editor
{
	void DrawTypesDebug(ast::Tree& ast)
	{
		if (!UI::CollapsingHeader("Types"))
		{
			return;
		}

		static const ImGuiTableFlags flags = ImGuiTableFlags_Reorderable | ImGuiTableFlags_Resizable
		                                   | ImGuiTableFlags_Hideable
		                                   | ImGuiTableFlags_SizingStretchProp;
		if (auto* types = ast.TryGetStatic<ast::STypes>())
		{
			UI::BeginChild("typesTableChild",
			    ImVec2(0.f, p::Min(250.f, UI::GetContentRegionAvail().y - 20.f)));
			if (UI::BeginTable("typesTable", 2, flags, ImVec2(0.f, UI::GetContentRegionAvail().y)))
			{
				UI::TableSetupColumn("Name");
				UI::TableSetupColumn("Id");
				UI::TableHeadersRow();

				for (const auto& it : types->typesByPath)
				{
					UI::TableNextRow();
					UI::TableNextColumn();    // Name
					UI::Text(p::ToString(it.first));

					UI::TableNextColumn();    // Id

					static p::String idText;
					idText.clear();
					p::Strings::FormatTo(idText, "{}", it.second);
					UI::Text(idText);
				}

				UI::EndTable();
			}
			UI::EndChild();
		}
	}


	ASTDebugger::ASTDebugger() {}

	void ASTDebugger::Draw(ast::Tree& ast)
	{
		if (!open)
		{
			return;
		}

		auto& dbgContext = ast.GetOrSetStatic<p::DebugContext>();
		dbgContext.ctx   = &ast;
		p::BeginDebug(dbgContext);
		p::DrawIdRegistry("  " ICON_FA_BUG "  AST Debugger", &open);
		static p::ECSDebugInspector testInspector;
		testInspector.id = mainInspector.id;
		p::DrawEntityInspector("Test", testInspector);
		p::EndDebug();
	}

	void ASTDebugger::OnInspectEntity(ast::Id id)
	{
		bool bOpenNewInspector = false;
		if (ImGui::GetIO().KeyCtrl)    // Inspector found and Ctrl? Open a new one
		{
			OpenAvailableSecondaryInspector(id);
		}
		else
		{
			bool wasInspected = secondaryInspectors.RemoveIf([id](const auto& inspector) {
				return inspector.id == id;
			}) > 0;
			if (mainInspector.id == id)
			{
				mainInspector.id = ast::NoId;
				wasInspected     = true;
			}

			if (!wasInspected)
			{
				mainInspector.id           = id;
				mainInspector.pendingFocus = true;
			}
		}
	}

	void ASTDebugger::DrawEntityInspector(p::StringView label, p::StringView id, ast::Tree& ast,
	    InspectorPanel& inspector, bool* open)
	{
		const bool valid   = ast.IsValid(inspector.id);
		const bool removed = ast.WasRemoved(inspector.id);
		bool clone         = false;
		ast::Id changedId  = inspector.id;

		p::String name;
		p::Strings::FormatTo(
		    name, "{}: {}{}###{}", label, inspector.id, removed ? " (removed)" : "", id);

		if (inspector.pendingFocus)
		{
			ImGui::SetNextWindowFocus();
			inspector.pendingFocus = false;
		}

		UI::SetNextWindowPos(ImGui::GetCursorScreenPos() + ImVec2(20, 20), ImGuiCond_Appearing);
		UI::SetNextWindowSizeConstraints(ImVec2(300.f, 200.f), ImVec2(800, FLT_MAX));
		UI::BeginOuterStyle();
		UI::PushTextColor(valid && !removed ? UI::whiteTextColor : UI::errorColor);
		ImGui::Begin(name.c_str(), open, ImGuiWindowFlags_MenuBar);
		UI::PopTextColor();

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu(ICON_FA_BARS))
			{
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Id");
				ImGui::SameLine();
				p::String asString = p::ToString(inspector.id);
				ImGui::SetNextItemWidth(100.f);
				if (UI::InputText("##IdValue", asString,
				        ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll))
				{
					changedId = p::IdFromString(asString);
				}
				ImGui::EndMenu();
			}

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - 40.f);
			if (ImGui::MenuItem("Clone"))
			{
				clone = true;
			}
			ImGui::EndMenuBar();
		}

		UI::BeginInnerStyle();

		if (valid)
		{
			for (const auto& poolInstance : ast.GetPools())
			{
				p::TypeId type = poolInstance.componentId;
				if (!type.IsValid() || !poolInstance.GetPool()->Has(inspector.id))
				{
					continue;
				}

				void* data = poolInstance.GetPool()->TryGetVoid(inspector.id);
				static p::String typeName;
				typeName = p::GetTypeName(type);

				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
				if (!data)
				{
					flags |= ImGuiTreeNodeFlags_Leaf;
				}
				if (UI::CollapsingHeader(typeName.c_str(), flags))
				{
					UI::Indent();
					if (p::HasTypeFlags(type, p::TF_Struct)
					    && UI::BeginInspector("EntityInspector"))
					{
						UI::InspectChildrenProperties({data, type});
						UI::EndInspector();
					}
					UI::Unindent();
				}
			}
		}
		UI::End();

		// Update after drawing
		if (changedId != inspector.id)
		{
			inspector.id = changedId;
		}

		if (clone)
		{
			OpenAvailableSecondaryInspector(inspector.id);
		}
	}

	void ASTDebugger::OpenAvailableSecondaryInspector(ast::Id id)
	{
		p::i32 availableIndex = secondaryInspectors.FindIndex([](const auto& inspector) {
			return !inspector.open || inspector.id == ast::NoId;
		});
		if (availableIndex != p::NO_INDEX)
		{
			secondaryInspectors[availableIndex] = {id};
		}
		else
		{
			secondaryInspectors.Add({id});
		}
	}
}    // namespace rift::editor
