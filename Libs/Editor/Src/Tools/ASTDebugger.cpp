// Copyright 2015-2023 Piperift - All rights reserved

#include "Tools/ASTDebugger.h"

#include <AST/Components/CStmtOutputs.h>
#include <AST/Statics/STypes.h>
#include <AST/Tree.h>
#include <AST/Utils/Namespaces.h>
#include <AST/Utils/Paths.h>
#include <IconsFontAwesome5.h>
#include <Pipe/ECS/Utils/Hierarchy.h>
#include <Pipe/Reflect/TypeRegistry.h>
#include <UI/Inspection.h>
#include <UI/UI.h>


namespace rift::Editor
{
	using namespace p::core;

	void DrawEntityInspector(AST::Tree& ast, AST::Id entityId, bool* open = nullptr)
	{
		String name = "Entity Inspector";
		if (!IsNone(entityId))
			Strings::FormatTo(name, " (id:{})", entityId);
		Strings::FormatTo(name, "###Entity Inspector");
		UI::Begin(name.c_str(), open);
		if (IsNone(entityId))
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
				auto* dataType = Cast<p::DataType>(type);
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
		if (auto* types = ast.TryGetStatic<AST::STypes>())
		{
			UI::BeginChild("typesTableChild",
			    ImVec2(0.f, p::math::Min(250.f, UI::GetContentRegionAvail().y - 20.f)));
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
					TArray<AST::Id> roots;
					p::ecs::GetRoots(access, roots);
					for (auto root : roots)
					{
						DrawNode(access, root, true);
					}

					TArray<AST::Id> orphans = p::ecs::ListAll<AST::CNamespace>(access);
					p::ecs::ExcludeIf<AST::CChild>(access, orphans);
					p::ecs::ExcludeIf<AST::CParent>(access, orphans);
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

		DrawEntityInspector(ast, selectedNode, &open);
	}

	void ASTDebugger::DrawNode(DrawNodeAccess access, AST::Id nodeId, bool showChildren)
	{
		static String idText;
		idText.clear();
		if (nodeId == AST::NoId)
		{
			idText = "No Id";
		}
		else if (auto version = p::ecs::GetVersion(nodeId); version > 0)
		{
			Strings::FormatTo(idText, "{} (v{})", p::ecs::GetIndex(nodeId), version);
		}
		else
		{
			Strings::FormatTo(idText, "{}", p::ecs::GetIndex(nodeId));
		}

		static String name;
		name.clear();
		if (const auto* id = access.TryGet<const AST::CNamespace>(nodeId))
		{
			name = id->name.AsString();
		}

		static String path;
		path.clear();
		if (const auto* file = access.TryGet<const AST::CFileRef>(nodeId))
		{
			path = p::ToString(file->path);

			StringView filename = p::GetFilename(path);
			Strings::FormatTo(name, name.empty() ? "file: {}" : " (file: {})", filename);
		}

		if (!filter.PassFilter(idText.c_str(), idText.c_str() + idText.size())
		    && !filter.PassFilter(name.c_str(), name.c_str() + name.size()))
		{
			return;
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		static String inspectLabel;
		inspectLabel.clear();
		Strings::FormatTo(inspectLabel, ICON_FA_SEARCH "##{}", nodeId);
		UI::PushButtonColor(UI::GetNeutralColor(1));
		UI::PushTextColor(
		    selectedNode == nodeId ? UI::whiteTextColor : UI::whiteTextColor.Translucency(0.3f));
		if (UI::Button(inspectLabel.c_str()))
		{
			selectedNode = nodeId;
		}
		UI::PopTextColor();
		UI::PopButtonColor();


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
		static Tag font{"WorkSans"};
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
}    // namespace rift::Editor
