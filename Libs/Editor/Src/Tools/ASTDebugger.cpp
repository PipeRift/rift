// Copyright 2015-2023 Piperift - All rights reserved

#include "Tools/ASTDebugger.h"

#include "imgui.h"
#include "UI/Widgets.h"

#include <AST/Components/Statements.h>
#include <AST/Statics/STypes.h>
#include <AST/Tree.h>
#include <AST/Utils/Namespaces.h>
#include <AST/Utils/Paths.h>
#include <IconsFontAwesome5.h>
#include <Pipe/Core/PlatformMisc.h>
#include <Pipe/Reflect/TypeRegistry.h>
#include <UI/Inspection.h>
#include <UI/UI.h>


namespace rift::Editor
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

		ImGui::SetNextWindowSize(ImVec2(400, 700), ImGuiCond_FirstUseEver);
		UI::Begin("  " ICON_FA_BUG "  Abstract Syntax Tree", &open);

		DrawTypesDebug(ast);

		if (UI::CollapsingHeader("Nodes"))
		{
			if (ImGui::BeginPopup("Options"))
			{
				ImGui::Checkbox("Show hierarchy", &showHierarchy);
				ImGui::EndPopup();
			}

			if (UI::Button("Options"))
			{
				UI::OpenPopup("Options");
			}
			UI::SameLine();
			filter.Draw("##Filter", -100.0f);


			static ImGuiTableFlags flags = ImGuiTableFlags_Reorderable | ImGuiTableFlags_Resizable
			                             | ImGuiTableFlags_Hideable
			                             | ImGuiTableFlags_SizingStretchProp;
			ImGui::BeginChild("nodesTableChild", {0.f, UI::GetContentRegionAvail().y - 20.f});
			if (UI::BeginTable("nodesTable", 5, flags))
			{
				UI::TableSetupColumn("", ImGuiTableColumnFlags_IndentDisable
				                             | ImGuiTableColumnFlags_WidthFixed
				                             | ImGuiTableColumnFlags_NoResize);    // Inspect
				UI::TableSetupColumn(
				    "Id", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_IndentEnable);
				UI::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 1.f);
				UI::TableSetupColumn("Path",
				    ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultHide, 1.2f);
				UI::TableSetupColumn("Namespace",
				    ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultHide, 1.2f);
				UI::TableHeadersRow();

				DrawNodeAccess access{ast};
				if (showHierarchy && !filter.IsActive())
				{
					p::TArray<ast::Id> roots;
					p::GetRootIds(access, roots);
					for (auto root : roots)
					{
						DrawNode(access, root, true);
					}

					p::TArray<ast::Id> orphans = p::FindAllIdsWith<ast::CNamespace>(access);
					p::ExcludeIdsWith<ast::CChild>(access, orphans);
					p::ExcludeIdsWith<ast::CParent>(access, orphans);
					for (auto orphan : orphans)
					{
						DrawNode(access, orphan, true);
					}
				}
				else
				{
					ast.Each([this, &access](ast::Id id) {
						DrawNode(access, id, false);
					});
				}
				UI::EndTable();
			}
			UI::EndChild();
			UI::Separator();
		}
		UI::End();

		// Inspectors
		DrawEntityInspector(
		    " " ICON_FA_LIST_ALT "  Inspector", "MainInspector", ast, mainInspector, nullptr);

		for (p::i32 i = 0; i < secondaryInspectors.Size(); ++i)
		{
			InspectorPanel& inspector = secondaryInspectors[i];
			if (inspector.open)
			{
				p::String id   = p::Strings::Format("SecondaryInspector{}", i);
				p::String name = p::Strings::Format(" " ICON_FA_LIST "  Inspector {}", i + 1);
				DrawEntityInspector(name, id, ast, inspector, &inspector.open);
			}
		}
	}

	void ASTDebugger::DrawNode(DrawNodeAccess access, ast::Id nodeId, bool showChildren)
	{
		static p::String idText;
		idText.clear();
		if (nodeId == ast::NoId)
		{
			idText = "No Id";
		}
		else if (auto version = p::GetIdVersion(nodeId); version > 0)
		{
			p::Strings::FormatTo(idText, "{} (v{})", p::GetIdIndex(nodeId), version);
		}
		else
		{
			p::Strings::FormatTo(idText, "{}", p::GetIdIndex(nodeId));
		}

		static p::String name;
		name.clear();
		if (const auto* id = access.TryGet<const ast::CNamespace>(nodeId))
		{
			name = id->name.AsString();
		}

		static p::String path;
		path.clear();
		if (const auto* file = access.TryGet<const ast::CFileRef>(nodeId))
		{
			path = p::ToString(file->path);

			p::StringView filename = p::GetFilename(path);
			p::Strings::FormatTo(name, name.empty() ? "file: {}" : " (file: {})", filename);
		}

		if (!filter.PassFilter(idText.c_str(), idText.c_str() + idText.size())
		    && !filter.PassFilter(name.c_str(), name.c_str() + name.size()))
		{
			return;
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		static p::String inspectLabel;
		inspectLabel.clear();

		const bool inspected =
		    mainInspector.id == nodeId || secondaryInspectors.Contains(InspectorPanel{nodeId});
		const char* icon = inspected ? ICON_FA_EYE : ICON_FA_EYE_SLASH;
		p::Strings::FormatTo(inspectLabel, "{}##{}", icon, nodeId);
		UI::PushTextColor(inspected ? UI::whiteTextColor : UI::whiteTextColor.Translucency(0.3f));
		UI::PushButtonColor(UI::GetNeutralColor(1));
		if (UI::Button(inspectLabel.c_str()))
		{
			OnInspectEntity(nodeId);
		}
		UI::PopButtonColor();
		UI::PopTextColor();


		ImGui::TableNextColumn();
		bool hasChildren;
		const ast::CParent* parent = nullptr;
		if (showChildren)
		{
			parent      = access.TryGet<const ast::CParent>(nodeId);
			hasChildren = parent && !parent->children.IsEmpty();
		}
		else
		{
			hasChildren = false;
		}

		bool open = false;
		static p::Tag font{"WorkSans"};
		UI::PushFont(font, UI::FontMode::Bold);
		if (hasChildren)
		{
			open = UI::TreeNodeEx(idText.c_str(), ImGuiTreeNodeFlags_SpanFullWidth);
		}
		else
		{
			UI::Indent(10.f);
			UI::Text(idText);
			UI::Unindent(10.f);
		}
		UI::PopFont();


		ImGui::TableNextColumn();
		UI::Text(name);


		if (ImGui::TableNextColumn())
		{
			UI::PushFont("WorkSans", UI::FontMode::Italic);
			UI::Text(path);
			UI::PopFont();
		}

		if (ImGui::TableNextColumn())
		{
			UI::Text(ast::GetParentNamespace(access, nodeId).ToString());
		}


		if (hasChildren && open)
		{
			for (ast::Id child : parent->children)
			{
				DrawNode(access, child, true);
			}

			UI::TreePop();
		}
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
			const auto& registry = ast::TypeRegistry::Get();
			for (const auto& poolInstance : ast.GetPools())
			{
				ast::Type* type = registry.FindType(poolInstance.componentId);
				if (!type || !poolInstance.GetPool()->Has(inspector.id))
				{
					continue;
				}

				void* data = poolInstance.GetPool()->TryGetVoid(inspector.id);
				static p::String typeName;
				typeName = type->GetName();

				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
				if (!data)
				{
					flags |= ImGuiTreeNodeFlags_Leaf;
				}
				if (UI::CollapsingHeader(typeName.c_str(), flags))
				{
					UI::Indent();
					auto* dataType = Cast<p::DataType>(type);
					if (data && dataType && UI::BeginInspector("EntityInspector"))
					{
						UI::InspectChildrenProperties({data, dataType});
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
}    // namespace rift::Editor
