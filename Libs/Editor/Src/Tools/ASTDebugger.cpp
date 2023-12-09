// Copyright 2015-2023 Piperift - All rights reserved

#include "Tools/ASTDebugger.h"

#include "imgui.h"
#include "UI/Widgets.h"

#include <AST/Components/CStmtOutputs.h>
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
	void DrawTypesDebug(AST::Tree& ast)
	{
		if (!UI::CollapsingHeader("Types"))
		{
			return;
		}

		static const ImGuiTableFlags flags = ImGuiTableFlags_Reorderable | ImGuiTableFlags_Resizable
		                                   | ImGuiTableFlags_Hideable
		                                   | ImGuiTableFlags_SizingStretchProp;
		if (auto* types = ast.TryGetStatic<AST::STypes>())
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

	void ASTDebugger::Draw(AST::Tree& ast)
	{
		if (!open)
		{
			return;
		}

		ImGui::SetNextWindowSize(ImVec2(400, 700), ImGuiCond_FirstUseEver);
		UI::Begin("Abstract Syntax Tree", &open);

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
					p::TArray<AST::Id> roots;
					p::GetRootIds(access, roots);
					for (auto root : roots)
					{
						DrawNode(access, root, true);
					}

					p::TArray<AST::Id> orphans = p::FindAllIdsWith<AST::CNamespace>(access);
					p::ExcludeIdsWith<AST::CChild>(access, orphans);
					p::ExcludeIdsWith<AST::CParent>(access, orphans);
					for (auto orphan : orphans)
					{
						DrawNode(access, orphan, true);
					}
				}
				else
				{
					ast.Each([this, &access](AST::Id id) {
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
		for (p::i32 i = 0; i < inspectorIds.Size(); ++i)
		{
			if (pendingInspectorFocusIndex == i)
			{
				ImGui::SetNextWindowFocus();
				pendingInspectorFocusIndex = p::NO_INDEX;
			}

			bool inspectorOpen = true;
			DrawEntityInspector(ast, inspectorIds[i], &inspectorOpen);
			if (!inspectorOpen)
			{
				inspectorIds.RemoveAtUnsafe(i, false);
				--i;
			}
		}
	}

	void ASTDebugger::DrawNode(DrawNodeAccess access, AST::Id nodeId, bool showChildren)
	{
		static p::String idText;
		idText.clear();
		if (nodeId == AST::NoId)
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
		if (const auto* id = access.TryGet<const AST::CNamespace>(nodeId))
		{
			name = id->name.AsString();
		}

		static p::String path;
		path.clear();
		if (const auto* file = access.TryGet<const AST::CFileRef>(nodeId))
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

		const bool inspected = inspectorIds.Contains(nodeId);
		const char* icon     = inspected ? ICON_FA_EYE : ICON_FA_EYE_SLASH;
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
		const AST::CParent* parent = nullptr;
		if (showChildren)
		{
			parent      = access.TryGet<const AST::CParent>(nodeId);
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
			UI::Text(AST::GetParentNamespace(access, nodeId).ToString());
		}


		if (hasChildren && open)
		{
			for (AST::Id child : parent->children)
			{
				DrawNode(access, child, true);
			}

			UI::TreePop();
		}
	}

	void ASTDebugger::OnInspectEntity(AST::Id id)
	{
		bool bOpenNewInspector = false;
		if (ImGui::GetIO().KeyCtrl)    // Inspector found and Ctrl? Open a new one
		{
			bOpenNewInspector = true;
		}
		else
		{
			p::i32 inspectorIndex = inspectorIds.FindIndex(id);
			if (inspectorIndex != p::NO_INDEX)    // At least one inspector has this entity
			{
				if (ImGui::GetIO().KeyCtrl)    // Inspector found and Ctrl? Open a new one
				{
					pendingInspectorFocusIndex = inspectorIndex;
				}
				else
				{
					inspectorIds.RemoveAtUnsafe(inspectorIndex, false);
				}
			}
			else    // Inspector not found
			{
				bOpenNewInspector = true;
			}
		}


		if (bOpenNewInspector)
		{
			inspectorIds.Add(id);
		}
	}

	void ASTDebugger::DrawEntityInspector(AST::Tree& ast, AST::Id entityId, bool* open)
	{
		const bool valid   = ast.IsValid(entityId);
		const bool removed = ast.WasRemoved(entityId);

		p::String name = "Inspector: ";
		p::Strings::FormatTo(name, "{0}{1}###{0}", entityId, removed ? " (removed)" : "");


		UI::SetNextWindowPos(ImGui::GetCursorScreenPos() + ImVec2(20, 20), ImGuiCond_Appearing);
		UI::SetNextWindowSizeConstraints(ImVec2(300.f, 200.f), ImVec2(800, FLT_MAX));
		UI::BeginOuterStyle();
		UI::PushTextColor(valid && !removed ? UI::whiteTextColor : UI::errorColor);
		ImGui::Begin(name.c_str(), open, ImGuiWindowFlags_MenuBar);
		UI::PopTextColor();
		UI::BeginInnerStyle();

		if (ImGui::BeginMenuBar())
		{
			// if (ImGui::MenuItem("Clone"))
			//{
			//	inspectorIds.Add(entityId);
			// }

			if (ImGui::BeginMenu("Settings"))
			{
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		if (!valid)
		{
			UI::End();
			return;
		}

		const auto& registry = AST::TypeRegistry::Get();
		for (const auto& poolInstance : ast.GetPools())
		{
			AST::Type* type = registry.FindType(poolInstance.componentId);
			if (!type || !poolInstance.GetPool()->Has(entityId))
			{
				continue;
			}

			void* data = poolInstance.GetPool()->TryGetVoid(entityId);
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
		UI::End();
	}
}    // namespace rift::Editor
