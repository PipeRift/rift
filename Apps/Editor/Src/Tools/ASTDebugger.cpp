// Copyright 2015-2021 Piperift - All rights reserved

#include "Tools/ASTDebugger.h"

#include <AST/Components/CChild.h>
#include <AST/Components/CFileRef.h>
#include <AST/Components/CIdentifier.h>
#include <AST/Components/CParent.h>
#include <AST/Tree.h>
#include <Framework/Paths.h>
#include <UI/UI.h>


namespace Rift
{
	using namespace Memory;

	ASTDebugger::ASTDebugger() {}

	void ASTDebugger::Draw(AST::Tree& ast)
	{
		if (!open)
		{
			return;
		}

		UI::Begin("Abstract Syntax Tree", &open);
		auto rootView   = ast.MakeView<CParent>(AST::TExclude<CChild>{});
		auto orphanView = ast.MakeView<CIdentifier>(AST::TExclude<CChild, CParent>{});

		static ImGuiTableFlags flags = ImGuiTableFlags_Reorderable | ImGuiTableFlags_Resizable |
		                               ImGuiTableFlags_Hideable | ImGuiTableFlags_SizingStretchProp;
		if (UI::BeginTable("entities", 3, flags, {0.f, UI::GetContentRegionAvail().y - 20.f}))
		{
			UI::TableSetupColumn("Id", ImGuiTableColumnFlags_NoHide);
			UI::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 1.f);
			UI::TableSetupColumn("Path",
			    ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultHide, 1.2f);
			UI::TableHeadersRow();

			for (auto root : rootView)
			{
				DrawEntity(ast, root);
			}

			for (auto orphan : orphanView)
			{
				DrawEntity(ast, orphan);
			}
			UI::EndTable();
		}
		UI::Separator();
		UI::End();
	}

	void ASTDebugger::DrawEntity(AST::Tree& ast, AST::Id entity)
	{
		static String idText;
		idText.clear();
		Strings::FormatTo(idText, "{}", entity);

		static String name;
		name.clear();
		if (CIdentifier* id = ast.TryGet<CIdentifier>(entity))
		{
			name = id->name.ToString();
		}

		static String path;
		path.clear();
		if (CFileRef* file = ast.TryGet<CFileRef>(entity))
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

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		const CParent* children = ast.TryGet<CParent>(entity);
		const bool hasChildren  = children && !children->children.IsEmpty();
		bool open               = false;
		static Name font{"WorkSans"};
		Style::PushFont(font, Style::FontMode::Bold);
		if (hasChildren)
		{
			open = UI::TreeNodeEx(idText.c_str(), ImGuiTreeNodeFlags_SpanFullWidth);
		}
		else
		{
			UI::Indent();
			UI::Text(idText.c_str());
			UI::Unindent();
		}
		Style::PopFont();
		ImGui::TableNextColumn();
		UI::Text(name.c_str());

		ImGui::TableNextColumn();
		Style::PushFont("WorkSans", Style::FontMode::Italic);
		UI::Text(path.c_str());
		Style::PopFont();

		if (hasChildren && open)
		{
			if (hasChildren)
			{
				for (AST::Id child : children->children)
				{
					DrawEntity(ast, child);
				}
			}
			UI::TreePop();
		}
	}
}    // namespace Rift
