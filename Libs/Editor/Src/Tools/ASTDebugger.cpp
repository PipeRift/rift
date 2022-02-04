// Copyright 2015-2022 Piperift - All rights reserved

#include "Tools/ASTDebugger.h"

#include <AST/Components/CChild.h>
#include <AST/Components/CFileRef.h>
#include <AST/Components/CIdentifier.h>
#include <AST/Components/CParent.h>
#include <AST/Components/CStatementOutputs.h>
#include <AST/Statics/STypes.h>
#include <AST/Tree.h>
#include <Framework/Paths.h>
#include <IconsFontAwesome5.h>
#include <Reflection/Registry/Registry.h>
#include <UI/ReflectionWidgets.h>
#include <UI/UI.h>


namespace Rift
{
	using namespace Memory;

	void DrawEntityInspector(AST::Tree& ast, AST::Id entityId, bool* open = nullptr)
	{
		const String name =
		    Strings::Format("Entity Inspector (id:{})###Entity Inspector", entityId);
		UI::Begin(name.c_str(), open);
		if (IsNone(entityId))
		{
			UI::End();
			return;
		}

		const auto& registry = Refl::ReflectionRegistry::Get();
		for (const auto& poolInstance : ast.GetPools())
		{
			Refl::Type* type = registry.FindType(poolInstance.componentId);
			if (!type || !poolInstance.GetPool()->Has(entityId))
			{
				continue;
			}

			void* data = poolInstance.GetPool()->TryGetVoid(entityId);
			static String typeName;
			typeName = type->GetName();

			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
			if (!data)
			{
				flags |= ImGuiTreeNodeFlags_Leaf;
			}
			if (UI::CollapsingHeader(typeName.c_str(), flags))
			{
				UI::Indent();
				Refl::DataType* dataType = type->AsData();
				if (data && dataType && UI::BeginInspector("EntityInspector"))
				{
					UI::InspectProperties(data, dataType);
					UI::EndInspector();
				}
				UI::Unindent();
			}
		}

		UI::End();
	}

	void DrawTypesDebug(AST::Tree& ast)
	{
		if (!UI::CollapsingHeader("Types"))
		{
			return;
		}

		static const ImGuiTableFlags flags = ImGuiTableFlags_Reorderable | ImGuiTableFlags_Resizable
		                                   | ImGuiTableFlags_Hideable
		                                   | ImGuiTableFlags_SizingStretchProp;
		if (auto* types = ast.TryGetStatic<STypes>())
		{
			UI::BeginChild("typesTableChild",
			    ImVec2(0.f, Math::Min(250.f, UI::GetContentRegionAvail().y - 20.f)));
			if (UI::BeginTable("typesTable", 2, flags, ImVec2(0.f, UI::GetContentRegionAvail().y)))
			{
				UI::TableSetupColumn("Name");
				UI::TableSetupColumn("Id");
				UI::TableHeadersRow();

				for (const auto& it : types->typesByName)
				{
					UI::TableNextRow();
					UI::TableNextColumn();    // Name
					UI::Text(it.first.ToString());

					UI::TableNextColumn();    // Id

					static String idText;
					idText.clear();
					Strings::FormatTo(idText, "{}", it.second);
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
			ImGui::BeginChild("nodesTableChild",
			    ImVec2(0.f, Math::Min(250.f, UI::GetContentRegionAvail().y - 20.f)));
			if (UI::BeginTable("nodesTable", 4, flags))
			{
				UI::TableSetupColumn("Id", ImGuiTableColumnFlags_NoHide);
				UI::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 1.f);
				UI::TableSetupColumn("Path",
				    ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultHide, 1.2f);
				UI::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed
				                             | ImGuiTableColumnFlags_NoResize);    // Inspect
				UI::TableHeadersRow();

				if (showHierarchy && !filter.IsActive())
				{
					auto roots = ast.Filter<CParent>(AST::TExclude<CChild>{});
					for (auto root : roots)
					{
						DrawNode(ast, root, true);
					}

					auto orphans = ast.Filter<CIdentifier>(AST::TExclude<CChild, CParent>{});
					for (auto orphan : orphans)
					{
						DrawNode(ast, orphan, true);
					}
				}
				else
				{
					ast.Each([this, &ast](AST::Id id) {
						DrawNode(ast, id, false);
					});
				}
				UI::EndTable();
			}
			UI::EndChild();
			UI::Separator();
		}
		UI::End();

		DrawEntityInspector(ast, selectedNode, &open);
	}

	void ASTDebugger::DrawNode(AST::Tree& ast, AST::Id nodeId, bool showChildren)
	{
		static String idText;
		idText.clear();
		Strings::FormatTo(idText, "{}", nodeId);

		static String name;
		name.clear();
		if (CIdentifier* id = ast.TryGet<CIdentifier>(nodeId))
		{
			name = id->name.ToString();
		}

		static String path;
		path.clear();
		if (CFileRef* file = ast.TryGet<CFileRef>(nodeId))
		{
			path = Paths::ToString(file->path);
			if (name.empty())
			{
				StringView filename = Paths::GetFilename(path);

				filename = Strings::RemoveFromEnd(filename, Paths::typeExtension);
				filename = Strings::RemoveFromEnd(filename, Paths::moduleExtension);
				Strings::FormatTo(name, "'{}'", filename);
			}
		}

		if (!filter.PassFilter(idText.c_str(), idText.c_str() + idText.size())
		    && !filter.PassFilter(name.c_str(), name.c_str() + name.size()))
		{
			return;
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		bool hasChildren;
		const CParent* children = nullptr;
		if (showChildren)
		{
			children    = ast.TryGet<CParent>(nodeId);
			hasChildren = children && !children->children.IsEmpty();
		}
		else
		{
			hasChildren = false;
		}

		bool open = false;
		static Name font{"WorkSans"};
		Style::PushFont(font, Style::FontMode::Bold);
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
		Style::PopFont();
		ImGui::TableNextColumn();
		UI::Text(name);

		ImGui::TableNextColumn();
		Style::PushFont("WorkSans", Style::FontMode::Italic);
		UI::Text(path);
		Style::PopFont();

		ImGui::TableNextColumn();
		static String inspectLabel;
		inspectLabel.clear();
		Strings::FormatTo(inspectLabel, ICON_FA_SEARCH "##{}", nodeId);
		if (UI::Button(inspectLabel.c_str()))
		{
			selectedNode = nodeId;
		}

		if (hasChildren && open)
		{
			for (AST::Id child : children->children)
			{
				DrawNode(ast, child, true);
			}

			UI::TreePop();
		}
	}
}    // namespace Rift
