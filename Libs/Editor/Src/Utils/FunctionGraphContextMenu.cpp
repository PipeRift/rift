// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Utils/FunctionGraphContextMenu.h"

#include "AST/Utils/TypeUtils.h"
#include "UI/Style.h"
#include "UI/Widgets.h"
#include "Utils/EditorStyle.h"
#include "Utils/FunctionGraph.h"
#include "Utils/Nodes.h"
#include "Utils/TypeUtils.h"
#include "Utils/Widgets.h"

#include <AST/Components/CNamespace.h>
#include <AST/Components/Declarations.h>
#include <AST/Components/Expressions.h>
#include <AST/Components/Views/CNodePosition.h>
#include <AST/Statics/STypes.h>
#include <AST/Systems/CastSystem.h>
#include <AST/Systems/FunctionsSystem.h>
#include <AST/Utils/Expressions.h>
#include <AST/Utils/Namespaces.h>
#include <AST/Utils/Statements.h>
#include <AST/Utils/TransactionUtils.h>
#include <GLFW/glfw3.h>
#include <IconsFontAwesome5.h>
#include <UI/UI.h>


namespace rift::editor::Graph
{
	// Expression pins carry a type, execution pins are the node itself and don't.
	static bool IsExprPin(ast::Tree& ast, Id pinId)
	{
		return ast.Has<ast::CExprType>(pinId) || ast.Has<ast::CExprTypeId>(pinId);
	}

	static bool IsTypeCompatible(ast::Tree& ast, Id valueType, Id targetType)
	{
		if (valueType == targetType)
		{
			return true;
		}
		// Pins without a type take the type of whatever gets connected to them
		if (IsNone(valueType) || IsNone(targetType))
		{
			return true;
		}
		return ast::CastSystem::CanCastImplicit(ast, valueType, targetType);
	}

	// First pin of the node the dropped link can connect to
	static Id FindMatchingPin(ast::Tree& ast, Id nodeId, const MenuContext& context)
	{
		const Id sourceType = GetPinType(ast, context.sourcePin);
		if (!IsNone(sourceType))
		{
			// Pins of a compatible type are used when no pin matches exactly
			Id compatiblePin = ast::NoId;
			if (context.type == Nodes::PinType::Output)
			{
				if (const auto* inputs = ast.TryGet<const ast::CExprInputs>(nodeId))
				{
					for (Id pinId : inputs->pinIds)
					{
						const Id pinType = GetPinType(ast, pinId);
						if (pinType == sourceType)
						{
							return pinId;
						}
						if (IsNone(compatiblePin) && IsTypeCompatible(ast, sourceType, pinType))
						{
							compatiblePin = pinId;
						}
					}
				}
			}
			else
			{
				if (const auto* outputs = ast.TryGet<const ast::CExprOutputs>(nodeId))
				{
					for (Id pinId : outputs->pinIds)
					{
						const Id pinType = GetPinType(ast, pinId);
						if (pinType == sourceType)
						{
							return pinId;
						}
						if (IsNone(compatiblePin) && IsTypeCompatible(ast, pinType, sourceType))
						{
							compatiblePin = pinId;
						}
					}
				}
			}
			return compatiblePin;
		}

		if (context.type == Nodes::PinType::Output && ast.Has<ast::CStmtInput>(nodeId))
		{
			return nodeId;
		}
		if (context.type == Nodes::PinType::Input)
		{
			if (ast.Has<ast::CStmtOutput>(nodeId))
			{
				return nodeId;
			}
			if (const auto* outputs = ast.TryGet<const ast::CStmtOutputs>(nodeId))
			{
				if (!outputs->pinIds.IsEmpty())
				{
					return outputs->pinIds[0];
				}
			}
		}
		return ast::NoId;
	}

	void SetPositionAndConnect(ast::Tree& ast, Id id, const MenuContext& context)
	{
		if (IsNone(id))
		{
			return;
		}

		ast.Add<CNodePosition>(id, settings.GetGridPosition(context.location).Floor());

		if (context.HasSource())
		{
			const Id targetPin = FindMatchingPin(ast, id, context);
			if (context.type == Nodes::PinType::Output)
			{
				ast::TryConnectStmt(ast, context.sourcePin, id);
				if (!IsNone(targetPin))
				{
					ast::TryConnectExpr(ast, ast::GetExprOutputFromPin(ast, context.sourcePin),
					    ast::GetExprInputFromPin(ast, targetPin));
				}
			}
			else if (!IsNone(targetPin))
			{
				ast::TryConnectStmt(ast, targetPin, context.sourcePin);
				ast::TryConnectExpr(ast, ast::GetExprOutputFromPin(ast, targetPin),
				    ast::GetExprInputFromPin(ast, context.sourcePin));
			}
		}
	}

	enum class ContextItemKind
	{
		Literal,
		Return,
		If,
		For,
		Function,
		Variable,
		VariableAssign,
		UnaryOperator,
		BinaryOperator,
		Cast,
	};

	// Assignments are statements, so they are created dragging from an execution output pin
	static bool IsExecutionContext(ast::Tree& ast, const MenuContext& context)
	{
		return context.HasSource() && !IsExprPin(ast, context.sourcePin)
		    && context.type == Nodes::PinType::Output;
	}

	// Whether a character is matched through when searching
	static bool IsSearchSeparator(char c)
	{
		return !p::TCharHelpers<char>::IsAlpha(c) && (c < '0' || c > '9');
	}

	// Whether text has a character that isn't a separator
	static bool HasSearchText(StringView text)
	{
		for (const char c : text)
		{
			if (!IsSearchSeparator(c))
			{
				return true;
			}
		}
		return false;
	}

	// An item of the context menu
	struct MenuEntry
	{
		ContextItemKind kind = ContextItemKind::Literal;
		// Function, variable or type the item represents
		Id id = ast::NoId;
		// Types of a cast entry
		ast::Cast cast{};
		// Operator of a unary or binary operator entry
		i32 operatorType = 0;
		// Name drawn in simple items. Operators format theirs when drawn
		StringView label;
		// Names that can match a search. The first one is the item's own name
		TArray<StringView, 4> keywords;
		// Background of the item's row, when it has one
		Color background = Color::Transparent();
		// Position in the cache, used to keep a stable draw order
		i32 order = 0;
		// Whether the item matches the current search
		bool isVisible = true;
		// How well the item matches the current search
		i32 score = 0;
	};

	// A labelled group of menu entries and subcategories
	struct MenuCategory
	{
		String label;
		// Category this one is drawn inside. -1 at the root
		i32 parent = -1;
		// Position in the cache, used to keep a stable draw order
		i32 order = 0;
		// Cached subcategories and entries
		TArray<i32> children;
		TArray<i32> entries;
		// Visible items and their best match score
		TArray<i32> visibleChildren;
		TArray<i32> visibleEntries;
		bool isVisible = true;
		i32 score      = 0;
	};

	// A parsed search: terms of words that must all match. Terms are alternatives, and a
	// leading '-' excludes a term's matches
	struct SearchQuery
	{
		TArray<TArray<StringView>> includes;
		TArray<TArray<StringView>> excludes;
	};

	// Entries of the context menu, cached in categories before drawing
	struct MenuCache
	{
		// Every entry is drawn under the hidden root category at index 0
		TArray<MenuCategory> categories;
		TArray<MenuEntry> entries;
		// Visible entries in menu order, ignoring categories
		TArray<i32> flatEntries;
		// First entry in menu order: the item Enter falls back to
		i32 defaultEntry = -1;
		// Item selected with the arrow keys, or -1 to select the first item drawn
		i32 selectedEntry = -1;
		// Last parsed search, and its parsed query
		String searchText;
		SearchQuery query;

		void Reset()
		{
			categories.Clear(p::Shrink::No);
			entries.Clear(p::Shrink::No);
			searchText.clear();
			query.includes.Clear();
			query.excludes.Clear();
			AddCategory(-1);
		}

		// Adds an empty category under 'parent', or a root category when 'parent' is -1
		i32 AddCategory(i32 parent)
		{
			const i32 categoryIdx  = categories.Size();
			MenuCategory& category = categories.AddRef();
			category.parent        = parent;
			category.order         = categoryIdx;
			if (parent != -1)
			{
				categories[parent].children.Add(categoryIdx);
			}
			return categoryIdx;
		}

		i32 AddEntry(MenuEntry&& entry, i32 category)
		{
			const i32 entryIdx = entries.Size();
			entry.order        = entryIdx;
			categories[category].entries.Add(entryIdx);
			entries.Add(p::Move(entry));
			return entryIdx;
		}
	};

	// Finds 'word' in 'text' ignoring case, or -1
	static i32 FindIgnoreCase(StringView text, StringView word)
	{
		const i32 textSize = i32(text.size());
		const i32 wordSize = i32(word.size());
		if (wordSize > textSize)
		{
			return -1;
		}
		for (i32 start = 0; start + wordSize <= textSize; ++start)
		{
			i32 i = 0;
			while (i < wordSize
			       && p::TCharHelpers<char>::ToLower(text[start + i])
			              == p::TCharHelpers<char>::ToLower(word[i]))
			{
				++i;
			}
			if (i == wordSize)
			{
				return start;
			}
		}
		return -1;
	}

	// Whether 'text' contains 'word', ignoring case. Separators are matched through, so
	// "print_line" finds "PrintLine". Words of symbols match literally
	static bool ContainsWord(StringView text, StringView word)
	{
		if (word.empty())
		{
			return true;
		}

		// Symbols aren't separators for matching
		const bool skipSeparators = HasSearchText(word);
		const i32 textSize        = i32(text.size());
		const i32 wordSize        = i32(word.size());
		for (i32 start = 0; start < textSize; ++start)
		{
			i32 textIdx  = start;
			i32 wordIdx  = 0;
			bool matched = true;
			while (wordIdx < wordSize)
			{
				if (skipSeparators)
				{
					while (textIdx < textSize && IsSearchSeparator(text[textIdx]))
					{
						++textIdx;
					}
					while (wordIdx < wordSize && IsSearchSeparator(word[wordIdx]))
					{
						++wordIdx;
					}
					if (wordIdx >= wordSize)
					{
						break;
					}
				}
				if (textIdx >= textSize
				    || p::TCharHelpers<char>::ToLower(text[textIdx])
				           != p::TCharHelpers<char>::ToLower(word[wordIdx]))
				{
					matched = false;
					break;
				}
				++textIdx;
				++wordIdx;
			}
			if (matched)
			{
				return true;
			}
		}
		return false;
	}

	// Splits text into its words on blanks
	static void ParseWords(StringView text, TArray<StringView>& outWords)
	{
		i32 wordStart = 0;
		for (i32 i = 0; i <= i32(text.size()); ++i)
		{
			if (i != i32(text.size()) && text[i] != ' ' && text[i] != '\t')
			{
				continue;
			}
			const StringView word = text.substr(wordStart, i - wordStart);
			wordStart             = i + 1;
			if (!word.empty())
			{
				outWords.Add(word);
			}
		}
	}

	// Parses search text like the search filter does: comma separated terms, with a
	// leading '-' to exclude. Words are views into 'search'
	static void ParseSearch(StringView search, SearchQuery& query)
	{
		query.includes.Clear();
		query.excludes.Clear();

		i32 termStart = 0;
		for (i32 i = 0; i <= i32(search.size()); ++i)
		{
			if (i != i32(search.size()) && search[i] != ',')
			{
				continue;
			}
			StringView term = search.substr(termStart, i - termStart);
			termStart       = i + 1;
			while (!term.empty() && (term.front() == ' ' || term.front() == '\t'))
			{
				term.remove_prefix(1);
			}

			bool excluded = false;
			if (!term.empty() && term.front() == '-')
			{
				excluded = true;
				term.remove_prefix(1);
			}

			TArray<StringView> words;
			ParseWords(term, words);
			if (!words.IsEmpty())
			{
				(excluded ? query.excludes : query.includes).Add(p::Move(words));
			}
		}
	}

	// Whether a term matches an entry: every word of the term must match one of its names
	static bool MatchesTerm(const TArray<StringView>& words, TView<StringView> keywords)
	{
		for (const StringView word : words)
		{
			bool found = false;
			for (const StringView keyword : keywords)
			{
				if (ContainsWord(keyword, word))
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				return false;
			}
		}
		return true;
	}

	// Whether an entry matches the search. Any term can match, excluded terms must not
	static bool MatchesSearch(const SearchQuery& query, TView<StringView> keywords)
	{
		for (const TArray<StringView>& words : query.excludes)
		{
			if (MatchesTerm(words, keywords))
			{
				return false;
			}
		}
		if (query.includes.IsEmpty())
		{
			return true;
		}
		for (const TArray<StringView>& words : query.includes)
		{
			if (MatchesTerm(words, keywords))
			{
				return true;
			}
		}
		return false;
	}

	// Scores of the different ways a search word can match a name
	constexpr i32 searchExactScore     = 400;
	constexpr i32 searchPrefixScore    = 300;
	constexpr i32 searchContainsScore  = 200;
	constexpr i32 searchSeparatorScore = 100;
	// Matches of secondary names rank below matches of the item's own name
	constexpr i32 searchKeywordPenalty = 150;

	// How well a word matches a name. Exact matches score the highest, followed by
	// prefixes, substrings and matches through separators
	static i32 ScoreWord(StringView name, StringView word)
	{
		const i32 found = FindIgnoreCase(name, word);
		if (found < 0)
		{
			return ContainsWord(name, word) ? searchSeparatorScore : 0;
		}
		if (i32(name.size()) == i32(word.size()))
		{
			return searchExactScore;
		}
		if (found == 0)
		{
			return searchPrefixScore;
		}
		return searchContainsScore - p::Min(found, 99);
	}

	// The best score of an entry against the search. Terms score the sum of their words
	static i32 ScoreSearch(const SearchQuery& query, TView<StringView> keywords)
	{
		i32 best = 0;
		for (const TArray<StringView>& words : query.includes)
		{
			i32 termScore = 0;
			for (const StringView word : words)
			{
				i32 wordScore = 0;
				for (i32 keywordIdx = 0; keywordIdx < keywords.Size(); ++keywordIdx)
				{
					const i32 penalty = keywordIdx > 0 ? searchKeywordPenalty : 0;
					wordScore = p::Max(wordScore, ScoreWord(keywords[keywordIdx], word) - penalty);
				}
				termScore += wordScore;
			}
			best = p::Max(best, termScore);
		}
		return best;
	}

	// Whether an item of the given kind fits the menu context
	static bool FitsContext(ast::Tree& ast, const MenuContext& context, ContextItemKind kind,
	    Id itemId = ast::NoId, const ast::Cast* cast = nullptr)
	{
		if (!context.HasSource())
		{
			return true;
		}

		const bool isExpr   = IsExprPin(ast, context.sourcePin);
		const bool isOutput = context.type == Nodes::PinType::Output;
		const Id sourceType = GetPinType(ast, context.sourcePin);

		switch (kind)
		{
			case ContextItemKind::Function:
				if (!isExpr)    // call nodes always have an stmt input and output
				{
					return true;
				}
				else if (isOutput)
				{
					if (const auto* inputs = ast.TryGet<const ast::CExprInputs>(itemId))
					{
						for (Id pinId : inputs->pinIds)
						{
							if (IsTypeCompatible(ast, sourceType, GetPinType(ast, pinId)))
							{
								return true;
							}
						}
					}
				}
				else
				{
					if (const auto* outputs = ast.TryGet<const ast::CExprOutputs>(itemId))
					{
						for (Id pinId : outputs->pinIds)
						{
							if (IsTypeCompatible(ast, GetPinType(ast, pinId), sourceType))
							{
								return true;
							}
						}
					}
				}
				break;
			case ContextItemKind::Literal:
				// Literals only produce a value, so they can only receive an input link
				return isExpr && context.type != Nodes::PinType::Output
				    && IsTypeCompatible(ast, itemId, sourceType);
			case ContextItemKind::Return: return false;
			case ContextItemKind::If:
				// if nodes have both an execution input and outputs, and only a bool
				// output fits the condition input
				return !isExpr || (isOutput && sourceType == ast.GetNativeTypes().boolId);
			case ContextItemKind::For:
				// for nodes can be connected from execution pins, or from integer
				// expressions to their "first"/"end" inputs and "index" output
				return !isExpr || sourceType == ast.GetNativeTypes().i32Id;
			case ContextItemKind::Variable:
				if (isExpr && !isOutput)
				{
					if (const auto* variable = ast.TryGet<const ast::CDeclVariable>(itemId))
					{
						return !IsNone(variable->typeId)
						    && IsTypeCompatible(ast, variable->typeId, sourceType);
					}
				}
				break;
			case ContextItemKind::VariableAssign:
				// Assignments are statements, so they need an execution context
				return !isExpr && isOutput;
			case ContextItemKind::UnaryOperator:
			case ContextItemKind::BinaryOperator: return isExpr;
			case ContextItemKind::Cast:
				// A cast takes the value in its input pin and produces it in its output pin
				if (!isExpr || !cast)
				{
					return false;    // Execution pins have no value to cast
				}
				if (IsNone(sourceType))
				{
					return false;
				}
				return isOutput ? IsTypeCompatible(ast, sourceType, cast->fromType)
				                : IsTypeCompatible(ast, cast->toType, sourceType);
		}
		return false;
	}

	bool ContextItem(
	    StringView name, Id id, Color background = Color::Transparent(), bool highlighted = false)
	{
		if (id != p::NoId)
		{
			UI::PushID(id);
		}
		constexpr float rounding = 2.f;

		// Keep the click area tight to the item so the 1px spacing between items shows
		ImGuiSelectableFlags flags = ImGuiSelectableFlags_NoPadWithHalfSpacing;
		if (highlighted)
		{
			flags |= ImGuiSelectableFlags_Highlight;    // Draw as if hovered
		}
		const bool hasBackground = background.a > 0.f;
		if (hasBackground)
		{
			UI::PushHeaderColor(background);
			if (highlighted)
			{
				// A highlighted row draws a clearly brighter version of its color
				UI::PushStyleColor(ImGuiCol_HeaderHovered, background.Tint(0.2f));
			}
		}

		UI::PushStyleVar(ImGuiStyleVar_ItemSpacing, v2{UI::GetStyle().ItemSpacing.x, 2.f});
		UI::PushStyleVar(ImGuiStyleVar_SelectableRounding, rounding);
		// Inset the text 4px from the background edges
		const float itemWidth  = UI::GetContentRegionAvail().x;
		const float textWidth  = UI::CalcTextSize(name).x;
		const float textAlignX = textWidth < itemWidth ? 4.f / (itemWidth - textWidth) : 0.f;
		UI::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, v2{textAlignX, 0.f});
		// A selected Selectable always paints its background
		const bool pressed = UI::Selectable(name, hasBackground, flags, v2{0.f, 16.f});
		UI::PopStyleVar(3);
		if (hasBackground)
		{
			if (highlighted)
			{
				UI::PopStyleColor();
			}
			UI::PopHeaderColor();
		}
		if (pressed)
		{
			UI::CloseCurrentPopup();
		}
		if (id != p::NoId)
		{
			UI::PopID();
		}
		return pressed;
	}

	// Draws a context menu row and closes the menu when it's picked. Row content is drawn
	// as chips over it, so the row must allow overlapping items
	bool ContextChipRow(StringView label, ImVec2& outMin, ImVec2& outMax,
	    Color background = Color::Transparent(), float height = 16.f, bool highlighted = false)
	{
		UI::SetNextItemAllowOverlap();
		const bool pressed =
		    UI::SelectableRow(label, outMin, outMax, background, height, highlighted);
		if (pressed)
		{
			UI::CloseCurrentPopup();
		}
		return pressed;
	}

	// A variable of the context menu: the icon of its action, followed by the variable as a
	// chip of its type
	bool ContextVariableItem(StringView name, Id variableId, const Color& color, bool isSetter)
	{
		UI::PushID(variableId);
		UI::PushID(isSetter);
		ImVec2 itemMin;
		ImVec2 itemMax;
		const bool pressed =
		    ContextChipRow("###Item", itemMin, itemMax, Color::Transparent(), 16.f, false);

		UI::SetCursorScreenPos(ImVec2{itemMin.x + UI::chipPadding, itemMin.y});
		UI::Chip(isSetter ? ICON_FA_SIGN_IN_ALT : ICON_FA_SIGN_OUT_ALT, Color::Transparent(),
		    nullptr, FLT_MAX, UI::ChipFlags::None);
		UI::SameLine(0.f, UI::chipSpacing);
		UI::Chip(name, color);
		UI::PopID();
		UI::PopID();

		UI::SetItemTooltip(
		    "%s value of variable %.*s", isSetter ? "Set" : "Get", i32(name.size()), name.data());
		return pressed;
	}

	// A cast of the context menu: each type carries a rounded background of its own color
	bool ContextCastItem(ast::Tree& ast, const ast::Cast& cast, bool highlighted = false)
	{
		const StringView fromName = ast::GetName(ast, cast.fromType).AsString();
		const StringView toName   = ast::GetName(ast, cast.toType).AsString();

		// Both types identify the cast in the menu
		UI::PushID(cast.fromType);
		UI::PushID(cast.toType);
		ImVec2 itemMin;
		ImVec2 itemMax;
		const bool pressed =
		    ContextChipRow("###Item", itemMin, itemMax, Color::Transparent(), 16.f, highlighted);

		UI::SetCursorScreenPos(ImVec2{itemMin.x + UI::chipPadding, itemMin.y});
		UI::Chip(fromName, GetTypeColor(ast, cast.fromType));
		UI::SameLine(0.f, UI::chipSpacing);
		UI::Chip(ICON_FA_ARROW_RIGHT, Color::Transparent(), nullptr, FLT_MAX, UI::ChipFlags::None);
		UI::SameLine(0.f, UI::chipSpacing);
		UI::Chip(toName, GetTypeColor(ast, cast.toType));
		UI::PopID();
		UI::PopID();

		UI::SetItemTooltip("Cast %.*s into %.*s", i32(fromName.size()), fromName.data(),
		    i32(toName.size()), toName.data());
		return pressed;
	}

	// A function entry: its name followed by its argument and return chips
	bool ContextFunctionItem(ast::Tree& ast, Id id, StringView name, bool highlighted = false)
	{
		UI::PushID(id);
		ImVec2 itemMin;
		ImVec2 itemMax;

		// The function carries its own background, slightly thicker than the pin chips
		const float rowHeight = UI::GetTextLineHeight() + 4.f;
		const bool pressed =
		    ContextChipRow("###Item", itemMin, itemMax, functionColor, rowHeight, highlighted);

		// Arguments produce a value inside the function, returns consume one
		const auto* args    = ast.TryGet<const ast::CExprOutputs>(id);
		const auto* returns = ast.TryGet<const ast::CExprInputs>(id);

		bool first   = true;
		auto drawPin = [&](Id pinId, bool isReturn)
		{
			if (!first)
			{
				UI::SameLine(0.f, UI::chipSpacing);
				UI::Chip(", ", Color::Transparent(), nullptr, FLT_MAX, UI::ChipFlags::None);
			}
			first = false;

			// Pin chip, leading with an arrow for returns
			const auto* ns = ast.TryGet<const ast::CNamespace>(pinId);
			UI::SameLine(0.f, UI::chipSpacing);
			UI::Chip(ns ? ns->name.AsString() : StringView{"none"},
			    GetTypeColor(ast, GetPinType(ast, pinId)),
			    isReturn ? ICON_FA_LONG_ARROW_ALT_UP : nullptr);
		};

		auto drawPins = [&](const p::TArray<Id>& pins, bool isReturn)
		{
			for (Id pinId : pins)
			{
				if (ast.IsValid(pinId))
				{
					drawPin(pinId, isReturn);
				}
			}
		};

		UI::SetCursorScreenPos(ImVec2{itemMin.x + UI::chipPadding, itemMin.y});
		UI::Chip(name, Color::Transparent(), nullptr, FLT_MAX, UI::ChipFlags::None);
		UI::SameLine(0.f, UI::chipSpacing);
		UI::Chip("(", Color::Transparent(), nullptr, FLT_MAX, UI::ChipFlags::None);
		if (args)
		{
			drawPins(args->pinIds, false);
		}
		if (returns)
		{
			drawPins(returns->pinIds, true);
		}
		UI::SameLine(0.f, UI::chipSpacing);
		UI::Chip(")", Color::Transparent(), nullptr, FLT_MAX, UI::ChipFlags::None);
		UI::PopID();
		return pressed;
	}

	// A constructor of the context menu: the type carries a rounded background of its color
	bool ContextConstructItem(
	    StringView typeName, Id id, const Color& color, bool highlighted = false)
	{
		UI::PushID(id);
		ImVec2 itemMin;
		ImVec2 itemMax;
		const bool pressed =
		    ContextChipRow("###Item", itemMin, itemMax, Color::Transparent(), 16.f, highlighted);

		UI::SetCursorScreenPos(ImVec2{itemMin.x + UI::chipPadding, itemMin.y});
		UI::Chip("Make ", Color::Transparent(), nullptr, FLT_MAX, UI::ChipFlags::None);
		UI::SameLine(0.f, UI::chipSpacing);
		UI::Chip(typeName, color);
		UI::PopID();

		UI::SetItemTooltip("Make a new value of %.*s", i32(typeName.size()), typeName.data());
		return pressed;
	}

	void DrawNodesContextMenu(ast::Tree& ast, Id typeId, TView<Id> nodeIds)
	{
		P_Check(!nodeIds.IsEmpty());
		const bool canEditBody = ast::HasFunctionBodies(ast, typeId);

		if (canEditBody && UI::MenuItem("Delete"))
		{
			ast::RemoveNodes(ast, nodeIds);
		}

		Id firstNodeId = nodeIds[0];
		if (nodeIds.Size() == 1)
		{
			if (ast.Has<ast::CDeclFunction>(firstNodeId))
			{
				if (canEditBody && UI::MenuItem("Add return node"))
				{
					Id newId = ast::AddReturn({ast, typeId});
					if (!IsNone(newId))
					{
						v2 position = ast.Get<CNodePosition>(firstNodeId).position;
						ast.Add<CNodePosition>(newId, position + v2{10.f, 0.f});

						ast::TryConnectStmt(ast, firstNodeId, newId);
					}
				}
			}
			else if (const auto* callId = ast.TryGet<const ast::CExprCallId>(firstNodeId))
			{
				if (UI::MenuItem("Show function"))
				{
					ShowFunction(ast, callId->functionId);
				}
			}
		}
		TArray<Id> calls = FindIdsWith<ast::CExprCall>(ast, nodeIds);
		if (!calls.IsEmpty() && UI::MenuItem("Refresh"))
		{
			ast.AddN<CMdfd<ast::CExprCallId>>(calls);
		}
	}

	void DrawLinksContextMenu(ast::Tree& ast, Id typeId, TView<Id> linkIds)
	{
		P_Check(!linkIds.IsEmpty());
		const bool canEditBody = ast::HasFunctionBodies(ast, typeId);

		Id firstLinkId = linkIds[0];

		if (canEditBody && UI::MenuItem("Delete"))
		{
			ScopedChange(ast, linkIds);
			for (Id linkId : linkIds)
			{
				ast::DisconnectExpr(ast, ast::GetExprInputFromPin(ast, firstLinkId));
				ast::DisconnectStmtLink(ast, firstLinkId);
			}
		}
	}

	// Icon shown before a type/file category name
	static StringView GetFileIcon(ast::Tree& ast, Id typeId)
	{
		if (ast.Has<ast::CModule>(typeId))
		{
			return ICON_FA_TH_LARGE;
		}
		const StringView typeIcon = GetTypeIcon(ast, typeId);
		return typeIcon.empty() ? ICON_FA_FILE : typeIcon;
	}

	// Collects the items that pass the filter in draw order: best matches first while
	// searching, and in cache order otherwise
	template<typename ItemType>
	static void CollectVisible(const TArray<i32>& itemIndices, const TArray<ItemType>& items,
	    TArray<i32>& outIndices, bool isSearching)
	{
		outIndices.Clear(p::Shrink::No);
		for (const i32 itemIdx : itemIndices)
		{
			if (items[itemIdx].isVisible)
			{
				outIndices.Add(itemIdx);
			}
		}
		if (isSearching)
		{
			outIndices.Sort([&items](i32 a, i32 b)
			{
				const ItemType& itemA = items[a];
				const ItemType& itemB = items[b];
				return itemA.score != itemB.score ? itemA.score > itemB.score
				                                  : itemA.order < itemB.order;
			});
		}
	}

	// Caches the functions and variables under a namespace
	static void AddNamespaceEntries(MenuCache& cache, ast::Tree& ast, const MenuContext& context,
	    const TArray<Id>& ids, i32 parent)
	{
		for (Id childId : ids)
		{
			const auto* ns = ast.TryGet<const ast::CNamespace>(childId);
			if (!ns)
			{
				continue;
			}
			const StringView childName = ns->name.AsString();

			if (ast.Has<ast::CDeclFunction>(childId))
			{
				if (!FitsContext(ast, context, ContextItemKind::Function, childId))
				{
					continue;
				}
				MenuEntry entry;
				entry.kind  = ContextItemKind::Function;
				entry.id    = childId;
				entry.label = childName;
				entry.keywords.Add(childName);
				cache.AddEntry(p::Move(entry), parent);
			}
			else if (ast.Has<ast::CDeclVariable>(childId))
			{
				const auto* variable   = ast.TryGet<const ast::CDeclVariable>(childId);
				const Color color      = GetTypeColor(ast, variable ? variable->typeId : ast::NoId);
				const bool isExecution = IsExecutionContext(ast, context);

				// Assignments are created from an execution pin or an empty canvas
				if (!context.HasSource() || isExecution)
				{
					if (FitsContext(ast, context, ContextItemKind::VariableAssign, childId))
					{
						MenuEntry entry;
						entry.kind       = ContextItemKind::VariableAssign;
						entry.id         = childId;
						entry.label      = childName;
						entry.background = color;
						entry.keywords.Add(childName);
						entry.keywords.Add("Set");
						cache.AddEntry(p::Move(entry), parent);
					}
				}
				if (!isExecution && FitsContext(ast, context, ContextItemKind::Variable, childId))
				{
					MenuEntry entry;
					entry.kind       = ContextItemKind::Variable;
					entry.id         = childId;
					entry.label      = childName;
					entry.background = color;
					entry.keywords.Add(childName);
					entry.keywords.Add("Get");
					cache.AddEntry(p::Move(entry), parent);
				}
			}
			else
			{
				TArray<Id> categoryChildren;
				p::GetIdChildren(ast, childId, categoryChildren);
				categoryChildren.RemoveIf([&ast](Id id)
				{
					return ast.Has<ast::CModule>(id);    // Modules are drawn at root
				}, p::Shrink::No);
				if (categoryChildren.IsEmpty())
				{
					continue;
				}

				const i32 category = cache.AddCategory(parent);
				p::FormatTo(cache.categories[category].label, "{} {}", GetFileIcon(ast, childId),
				    childName);
				AddNamespaceEntries(cache, ast, context, categoryChildren, category);
			}
		}
	}

	// Caches the entries of an operator kind
	template<typename EnumType>
	static void AddOperatorEntries(MenuCache& cache, ast::Tree& ast, const MenuContext& context,
	    i32 category, ContextItemKind kind, StringView (*getName)(EnumType),
	    StringView (*getLongName)(EnumType))
	{
		if (!FitsContext(ast, context, kind))
		{
			return;    // Operators of this kind don't fit the menu context
		}
		for (const EnumType type : GetEnumValues<EnumType>())
		{
			MenuEntry entry;
			entry.kind         = kind;
			entry.operatorType = i32(type);
			entry.keywords.Add(getName(type));
			entry.keywords.Add(getLongName(type));
			cache.AddEntry(p::Move(entry), category);
		}
	}

	// Caches the flow, operator, constructor and cast entries of the menu
	static void AddBuiltinEntries(MenuCache& cache, ast::Tree& ast, const MenuContext& context)
	{
		const i32 flow               = cache.AddCategory(0);
		cache.categories[flow].label = "Flow";
		if (FitsContext(ast, context, ContextItemKind::Return))
		{
			MenuEntry entry;
			entry.kind  = ContextItemKind::Return;
			entry.label = "Return";
			entry.keywords.Add("Return");
			cache.AddEntry(p::Move(entry), flow);
		}
		if (FitsContext(ast, context, ContextItemKind::If))
		{
			MenuEntry entry;
			entry.kind  = ContextItemKind::If;
			entry.label = "If";
			entry.keywords.Add("If");
			cache.AddEntry(p::Move(entry), flow);
		}
		if (FitsContext(ast, context, ContextItemKind::For))
		{
			MenuEntry entry;
			entry.kind  = ContextItemKind::For;
			entry.label = "For";
			entry.keywords.Add("For");
			cache.AddEntry(p::Move(entry), flow);
		}

		const i32 operators               = cache.AddCategory(0);
		cache.categories[operators].label = "Operators";
		AddOperatorEntries(cache, ast, context, operators, ContextItemKind::UnaryOperator,
		    &GetUnaryOperatorName, &GetUnaryOperatorLongName);
		AddOperatorEntries(cache, ast, context, operators, ContextItemKind::BinaryOperator,
		    &GetBinaryOperatorName, &GetBinaryOperatorLongName);

		const i32 constructors               = cache.AddCategory(0);
		cache.categories[constructors].label = "Constructors";
		{
			TIdScope<ast::CDeclType, ast::CNamespace> typesScope{ast};
			for (Id literalTypeId : FindAllIdsWith<ast::CDeclType>(typesScope))
			{
				const auto* ns = typesScope.TryGet<const ast::CNamespace>(literalTypeId);
				if (!ns || !FitsContext(ast, context, ContextItemKind::Literal, literalTypeId))
				{
					continue;
				}
				const StringView typeName = ns->name.AsString();
				MenuEntry entry;
				entry.kind       = ContextItemKind::Literal;
				entry.id         = literalTypeId;
				entry.label      = typeName;
				entry.background = GetTypeColor(ast, literalTypeId);
				entry.keywords.Add("Make");
				entry.keywords.Add(typeName);
				cache.AddEntry(p::Move(entry), constructors);
			}
		}

		const i32 casts               = cache.AddCategory(0);
		cache.categories[casts].label = "Casts";
		for (const ast::Cast& cast : ast::CastSystem::GetCasts(ast))
		{
			if (!FitsContext(ast, context, ContextItemKind::Cast, ast::NoId, &cast))
			{
				continue;
			}
			MenuEntry entry;
			entry.kind = ContextItemKind::Cast;
			entry.cast = cast;
			entry.keywords.Add("Cast");
			entry.keywords.Add(ast::GetName(ast, cast.fromType).AsString());
			entry.keywords.Add(ast::GetName(ast, cast.toType).AsString());
			cache.AddEntry(p::Move(entry), casts);
		}
	}

	// Caches every entry of the menu before drawing
	static void BuildMenuCache(MenuCache& cache, ast::Tree& ast, const MenuContext& context)
	{
		cache.Reset();
		AddBuiltinEntries(cache, ast, context);
		// Functions and variables, grouped under their module and type namespaces
		AddNamespaceEntries(
		    cache, ast, context, p::FindAllIdsWith<ast::CModule, ast::CNamespace>(ast), 0);
	}

	// Collects the visible entries of a category in order
	static void CollectFlatEntries(
	    const MenuCache& cache, i32 categoryIdx, TArray<i32>& outEntryIndices)
	{
		const MenuCategory& category = cache.categories[categoryIdx];
		for (const i32 entryIdx : category.visibleEntries)
		{
			outEntryIndices.Add(entryIdx);
		}
		for (const i32 childIdx : category.visibleChildren)
		{
			CollectFlatEntries(cache, childIdx, outEntryIndices);
		}
	}

	// Filters and scores the cached entries and orders the categories
	static void FilterMenuCache(MenuCache& cache, const ImGuiTextFilter& filter)
	{
		// Parse the search only when it changes
		const StringView search = filter.InputBuf;
		if (search != cache.searchText)
		{
			cache.searchText.clear();
			cache.searchText += search;
			ParseSearch(cache.searchText.AsView(), cache.query);
		}

		const bool isSearching = filter.IsActive();
		for (MenuEntry& entry : cache.entries)
		{
			entry.isVisible = !isSearching || MatchesSearch(cache.query, entry.keywords);
			entry.score     = entry.isVisible ? ScoreSearch(cache.query, entry.keywords) : 0;
		}

		// Categories collect their contents bottom-up, so children are scored first
		for (i32 categoryIdx = cache.categories.Size() - 1; categoryIdx >= 0; --categoryIdx)
		{
			MenuCategory& category = cache.categories[categoryIdx];
			CollectVisible(category.entries, cache.entries, category.visibleEntries, isSearching);
			CollectVisible(
			    category.children, cache.categories, category.visibleChildren, isSearching);

			category.isVisible =
			    !category.visibleEntries.IsEmpty() || !category.visibleChildren.IsEmpty();
			category.score = 0;
			for (const i32 entryIdx : category.visibleEntries)
			{
				category.score = p::Max(category.score, cache.entries[entryIdx].score);
			}
			for (const i32 childIdx : category.visibleChildren)
			{
				category.score = p::Max(category.score, cache.categories[childIdx].score);
			}
		}

		cache.flatEntries.Clear(p::Shrink::No);
		CollectFlatEntries(cache, 0, cache.flatEntries);
		cache.defaultEntry = cache.flatEntries.IsEmpty() ? -1 : cache.flatEntries[0];
	}

	// Spawns the node of a picked entry, attached to the edited type
	static void SpawnEntryNode(
	    ast::Tree& ast, Id typeId, const MenuContext& context, const MenuEntry& entry)
	{
		Id newId = ast::NoId;
		switch (entry.kind)
		{
			case ContextItemKind::Return: newId = ast::AddReturn({ast, typeId}); break;
			case ContextItemKind::If: newId = ast::AddIf({ast, typeId}); break;
			case ContextItemKind::For: newId = ast::AddFor({ast, typeId}); break;
			case ContextItemKind::UnaryOperator:
				newId = ast::AddUnaryOperator(
				    {ast, typeId}, static_cast<ast::UnaryOperatorType>(entry.operatorType));
				break;
			case ContextItemKind::BinaryOperator:
				newId = ast::AddBinaryOperator(
				    {ast, typeId}, static_cast<ast::BinaryOperatorType>(entry.operatorType));
				break;
			case ContextItemKind::Function:
				newId = ast::AddCall({ast, typeId}, entry.id);
				// Populate the new call's pins right away so the dropped link can find
				// a matching pin to connect to
				ast::FunctionsSystem::SyncCallPinsFromFunction(ast);
				break;
			case ContextItemKind::Variable:
				// Attach to the edited type, not the variable's owning type
				newId = ast::AddDeclarationReference({ast, typeId}, entry.id);
				break;
			case ContextItemKind::VariableAssign:
				newId = ast::AddAssign({ast, typeId}, entry.id);
				break;
			case ContextItemKind::Literal:
				// Attach to the edited type, not the literal's own type
				newId = ast::AddLiteral({ast, typeId}, entry.id);
				break;
			case ContextItemKind::Cast:
				newId = ast::AddExplicitCast({ast, typeId}, entry.cast.fromType, entry.cast.toType);
				break;
		}
		SetPositionAndConnect(ast, newId, context);
	}

	// Draws a cached entry and spawns its node when picked
	static void DrawMenuEntry(ast::Tree& ast, Id typeId, const MenuContext& context,
	    const MenuEntry& entry, bool highlighted)
	{
		StringView label = entry.label;
		if (entry.kind == ContextItemKind::UnaryOperator
		    || entry.kind == ContextItemKind::BinaryOperator)
		{
			// Operators show both of their names together
			static String operatorLabel;
			operatorLabel.clear();
			p::FormatTo(operatorLabel, "{}   ({})", entry.keywords[0], entry.keywords[1]);
			label = operatorLabel.AsView();
		}

		bool pressed = false;
		switch (entry.kind)
		{
			case ContextItemKind::Function:
				pressed = ContextFunctionItem(ast, entry.id, label, highlighted);
				break;
			case ContextItemKind::Literal:
				pressed = ContextConstructItem(label, entry.id, entry.background, highlighted);
				break;
			case ContextItemKind::Cast:
				pressed = ContextCastItem(ast, entry.cast, highlighted);
				break;
			case ContextItemKind::Variable:
				pressed = ContextVariableItem(label, entry.id, entry.background, false);
				break;
			case ContextItemKind::VariableAssign:
				pressed = ContextVariableItem(label, entry.id, entry.background, true);
				break;
			case ContextItemKind::Return:
			case ContextItemKind::If:
			case ContextItemKind::For:
			case ContextItemKind::UnaryOperator:
			case ContextItemKind::BinaryOperator:
				pressed = ContextItem(label, entry.id, entry.background, highlighted);
				break;
		}
		if (pressed)
		{
			SpawnEntryNode(ast, typeId, context, entry);
		}
	}

	// Scrolls the item just drawn into view, with a small margin so the next items ahead
	// stay visible
	static void ScrollLastItemIntoView()
	{
		ImRect itemRect(UI::GetItemRectMin(), UI::GetItemRectMax());
		itemRect.Expand(ImVec2(0.f, UI::GetTextLineHeight()));
		UI::ScrollToRectEx(UI::GetCurrentWindow(), itemRect, ImGuiScrollFlags_KeepVisibleEdgeY);
	}

	static void DrawMenuCategory(ast::Tree& ast, Id typeId, const MenuContext& context,
	    MenuCache& cache, i32 categoryIdx, bool showSelection, bool& scrollToSelection);

	// Draws the entries and subcategories of a category
	static void DrawMenuCategoryContents(ast::Tree& ast, Id typeId, const MenuContext& context,
	    MenuCache& cache, const MenuCategory& category, bool showSelection, bool& scrollToSelection)
	{
		for (const i32 entryIdx : category.visibleEntries)
		{
			if (cache.selectedEntry == -1)
			{
				cache.selectedEntry = entryIdx;
			}
			const bool isSelected = entryIdx == cache.selectedEntry;
			DrawMenuEntry(
			    ast, typeId, context, cache.entries[entryIdx], showSelection && isSelected);
			if (scrollToSelection && isSelected)
			{
				ScrollLastItemIntoView();
				scrollToSelection = false;
			}
		}
		for (const i32 childIdx : category.visibleChildren)
		{
			DrawMenuCategory(
			    ast, typeId, context, cache, childIdx, showSelection, scrollToSelection);
		}
	}

	// Draws a cached category and its contents, skipping empty ones
	static void DrawMenuCategory(ast::Tree& ast, Id typeId, const MenuContext& context,
	    MenuCache& cache, i32 categoryIdx, bool showSelection, bool& scrollToSelection)
	{
		const MenuCategory& category = cache.categories[categoryIdx];
		if (category.visibleEntries.IsEmpty() && category.visibleChildren.IsEmpty())
		{
			return;
		}

		// Categories open by default, unless the user opened or closed them before
		UI::SetNextItemOpen(true, ImGuiCond_Once);
		if (!UI::TreeNode(category.label.c_str()))
		{
			return;
		}
		DrawMenuCategoryContents(
		    ast, typeId, context, cache, category, showSelection, scrollToSelection);
		UI::TreePop();
	}

	void DrawGraphContextMenu(ast::Tree& ast, Id typeId, MenuContext context)
	{
		static ImGuiTextFilter filter;
		static MenuCache cache;
		const bool canEditBody = ast::HasFunctionBodies(ast, typeId);

		const bool isAppearing = UI::IsWindowAppearing();
		if (isAppearing)
		{
			UI::SetKeyboardFocusHere();
			filter.Clear();
		}
		const bool filterChanged = filter.Draw("##Filter");
		const bool filterActive  = UI::IsItemActive();
		// The search input deactivates itself when Enter is pressed, so keep treating that
		// frame as active: Enter still applies the selected item
		const bool filterEditing = filterActive || UI::IsItemDeactivated();
		// The menu draws its own selection, so no nav focus rectangles
		UI::SetNavCursorVisible(false);
		if (filterActive)
		{
			// Move the selection instead of navigating, so the search stays writable
			UI::GetCurrentContext()->ActiveIdUsingNavDirMask |=
			    (1 << ImGuiDir_Up) | (1 << ImGuiDir_Down);
		}

		// Spawn at the release position when the menu came from a dropped link, so the
		// new node lands where the link was let go
		if (!context.HasSource())
		{
			context.location = UI::GetMousePosOnOpeningCurrentPopup();
		}

		if (!canEditBody)
		{
			return;
		}

		// Entries are cached once, when the menu opens
		if (isAppearing)
		{
			BuildMenuCache(cache, ast, context);
		}

		// The first item drawn becomes the selected one when the results change
		const bool focusDefault = isAppearing || filterChanged;

		// Only the results scroll, so the search bar stays visible. The results take the rest
		// of the menu height, under the search row
		const float maxHeight        = p::Max(UI::GetIO().DisplaySize.y * 0.3f, 1.f);
		const float resultsMaxHeight = p::Max(
		    maxHeight - UI::GetStyle().WindowPadding.y * 2.f - UI::GetFrameHeightWithSpacing(),
		    1.f);
		UI::SetNextWindowSizeConstraints(v2{0.f, 0.f}, v2{FLT_MAX, resultsMaxHeight});
		UI::BeginChild(
		    "##Results", v2{0.f, 0.f}, ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_NavFlattened);
		{
			// Filtering and scoring run as a previous step to drawing
			FilterMenuCache(cache, filter);

			// The first item drawn becomes the selected one when the results change
			bool scrollToSelection = focusDefault;
			if (focusDefault)
			{
				cache.selectedEntry = -1;
			}

			// While the search input is active, up and down move the selected item. Holding
			// shift moves five items at a time
			if (filterActive && !cache.flatEntries.IsEmpty())
			{
				const bool fastMove =
				    UI::IsKeyDown(ImGuiKey_LeftShift) || UI::IsKeyDown(ImGuiKey_RightShift);
				const i32 step   = fastMove ? 5 : 1;
				i32 selectionIdx = cache.flatEntries.FindIndex(cache.selectedEntry);
				if (UI::IsKeyPressed(ImGuiKey_DownArrow))
				{
					selectionIdx += step;
				}
				if (UI::IsKeyPressed(ImGuiKey_UpArrow))
				{
					selectionIdx -= step;
				}
				selectionIdx            = p::Clamp(selectionIdx, 0, cache.flatEntries.Size() - 1);
				const i32 selectedEntry = cache.flatEntries[selectionIdx];
				scrollToSelection |= selectedEntry != cache.selectedEntry;
				cache.selectedEntry = selectedEntry;
			}

			// The hidden root category only wraps the contents of the menu
			DrawMenuCategoryContents(
			    ast, typeId, context, cache, cache.categories[0], filterActive, scrollToSelection);

			// Enter applies the selected item, or the default one when nothing is selected
			if (filterEditing && UI::IsKeyPressed(ImGuiKey_Enter))
			{
				const i32 entryToSpawn =
				    cache.selectedEntry != -1 ? cache.selectedEntry : cache.defaultEntry;
				if (entryToSpawn != -1)
				{
					SpawnEntryNode(ast, typeId, context, cache.entries[entryToSpawn]);
					UI::CloseCurrentPopup();
				}
			}
		}
		UI::EndChild();
	}

	void DrawContextMenu(ast::Tree& ast, Id typeId, MenuContext context)
	{
		// Cap the height so a long menu scrolls instead of overflowing the display
		const float maxHeight = p::Max(UI::GetIO().DisplaySize.y * 0.3f, 1.f);
		UI::SetNextWindowSizeConstraints(ImVec2(0.f, 0.f), ImVec2(FLT_MAX, maxHeight));
		if (UI::BeginPopup("ContextMenu"))
		{
			if (!IsNone(context.hoveredNodeId))
			{
				if (Nodes::IsNodeSelected(context.hoveredNodeId))
				{
					DrawNodesContextMenu(ast, typeId, Nodes::GetSelectedNodes());
				}
				else
				{
					DrawNodesContextMenu(ast, typeId, context.hoveredNodeId);
				}
			}
			else if (!IsNone(context.hoveredLinkId))
			{
				TArray<Id> selectedLinkIds;
				if (Nodes::GetSelectedLinks(selectedLinkIds))
				{
					DrawLinksContextMenu(ast, typeId, selectedLinkIds);
				}
				else
				{
					DrawLinksContextMenu(ast, typeId, context.hoveredLinkId);
				}
			}
			else
			{
				DrawGraphContextMenu(ast, typeId, context);
			}
			UI::EndPopup();
		}
	}
}    // namespace rift::editor::Graph
