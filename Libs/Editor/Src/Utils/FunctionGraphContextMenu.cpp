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

	// Can a value of 'valueType' be used where a value of 'targetType' is expected?
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

	// The first pin on 'nodeId' that the dropped link can connect to. For expression links
	// it's the first pin with a matching type, for execution links the node's exec pin.
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
		Function,
		Variable,
		UnaryOperator,
		BinaryOperator,
		Cast,
	};

	// A name that can match a search: the lowercased name and its standardized form
	struct SearchKeyword
	{
		String name;
		String standardized;
	};

	// Whether a character stays in standardized search text
	static bool IsSearchAlphanumeric(char c)
	{
		return p::TCharHelpers<char>::IsAlpha(c) || (c >= '0' && c <= '9');
	}

	// Lowercases text so searches match regardless of case
	static String LowercaseSearchText(StringView text)
	{
		String lowercased;
		for (const char c : text)
		{
			lowercased.push_back(p::TCharHelpers<char>::ToLower(c));
		}
		return lowercased;
	}

	// Standardizes text to match searches: lowercased and without separators. Lets a
	// search for "Print Line" also match "PrintLine", "print_line" or "print-line"
	static String StandardizeSearchText(StringView text)
	{
		String standardized;
		for (const char c : text)
		{
			if (IsSearchAlphanumeric(c))
			{
				standardized.push_back(p::TCharHelpers<char>::ToLower(c));
			}
		}
		return standardized;
	}

	// An item of the context menu. Entries are cached before drawing so filtering and
	// scoring run as a fast step over all of them
	struct MenuEntry
	{
		ContextItemKind kind = ContextItemKind::Literal;
		// Function, variable or type the item represents
		Id id = ast::NoId;
		// Types of a cast entry
		ast::Cast cast{};
		// Operator of a unary or binary operator entry
		i32 operatorType = 0;
		// Name drawn in simple items
		String label;
		// Names that can match a search. The first one is the item's own name
		TArray<SearchKeyword> keywords;
		// Background of the item's row, when it has one
		Color background = Color::Transparent();
		// Position in the cache, used to keep a stable draw order
		i32 order = 0;
		// Whether the item matches the current search. Updated while filtering
		bool isVisible = true;
		// How well the item matches the current search. Updated while filtering
		i32 score = 0;

		// Adds a name that can match a search
		void AddKeyword(StringView name)
		{
			SearchKeyword& keyword = keywords.AddRef();
			keyword.name           = LowercaseSearchText(name);
			keyword.standardized   = StandardizeSearchText(name);
		}
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
		// Visible items and their best match score. Updated while filtering
		TArray<i32> visibleChildren;
		TArray<i32> visibleEntries;
		bool isVisible = true;
		i32 score      = 0;
	};

	// Entries of the context menu, cached in categories before drawing
	struct MenuCache
	{
		// Every entry is drawn under the hidden root category at index 0
		TArray<MenuCategory> categories;
		TArray<MenuEntry> entries;
		// Visible entries in menu order, ignoring categories. Updated while filtering
		TArray<i32> flatEntries;
		// First entry in menu order: the item Enter falls back to. Updated while filtering
		i32 defaultEntry = -1;
		// Item selected with the arrow keys, or -1 to select the first item drawn
		i32 selectedEntry = -1;

		void Reset()
		{
			categories.Clear(p::Shrink::No);
			entries.Clear(p::Shrink::No);
			AddCategory({}, -1);
		}

		// Adds a category under 'parent', or a root category when 'parent' is -1
		i32 AddCategory(StringView label, i32 parent)
		{
			const i32 categoryIdx  = categories.Size();
			MenuCategory& category = categories.AddRef();
			category.label         = String{label};
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

	// A word of a search term: the lowercased word and its standardized form
	struct SearchWord
	{
		String name;
		String standardized;
	};

	// A parsed search: terms of words that must all match. Terms are alternatives, and a
	// '-' before a term excludes its matches from the results
	struct SearchQuery
	{
		TArray<TArray<SearchWord>> includes;
		TArray<TArray<SearchWord>> excludes;
	};

	// Splits search text into words on its blanks
	static TArray<SearchWord> ParseSearchWords(StringView text)
	{
		TArray<SearchWord> words;
		i32 wordStart = 0;
		for (i32 i = 0; i <= i32(text.size()); ++i)
		{
			if (i != i32(text.size()) && text[i] != ' ' && text[i] != '\t')
			{
				continue;
			}
			const StringView wordText = text.substr(wordStart, i - wordStart);
			wordStart                 = i + 1;
			if (wordText.empty())
			{
				continue;
			}
			SearchWord& word  = words.AddRef();
			word.name         = LowercaseSearchText(wordText);
			word.standardized = StandardizeSearchText(wordText);
		}
		return words;
	}

	// Parses search text like the search filter does: comma separated terms, with a
	// leading '-' to exclude
	static SearchQuery ParseSearch(StringView search)
	{
		SearchQuery query;
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
			while (!term.empty() && (term.back() == ' ' || term.back() == '\t'))
			{
				term.remove_suffix(1);
			}

			bool isExcluded = false;
			if (!term.empty() && term.front() == '-')
			{
				isExcluded = true;
				term.remove_prefix(1);
			}

			TArray<SearchWord> words = ParseSearchWords(term);
			if (words.IsEmpty())
			{
				continue;
			}
			(isExcluded ? query.excludes : query.includes).Add(p::Move(words));
		}
		return query;
	}

	// Whether a search word matches a name: a substring of the name, or of its
	// standardized form. Words of symbols match the name directly
	static bool MatchesSearchWord(const SearchKeyword& keyword, const SearchWord& word)
	{
		if (p::Strings::Contains(keyword.name, word.name))
		{
			return true;
		}
		return !word.standardized.empty() && !keyword.standardized.empty()
		    && p::Strings::Contains(keyword.standardized, word.standardized);
	}

	// Whether a search term matches an entry: every word of the term must match one of
	// its names
	static bool MatchesSearchTerm(
	    const TArray<SearchWord>& words, const TArray<SearchKeyword>& keywords)
	{
		for (const SearchWord& word : words)
		{
			bool found = false;
			for (const SearchKeyword& keyword : keywords)
			{
				if (MatchesSearchWord(keyword, word))
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

	// Whether an entry matches the search: any term can match, all its words must match,
	// and excluded terms must not match
	static bool MatchesSearch(const SearchQuery& query, const TArray<SearchKeyword>& keywords)
	{
		for (const TArray<SearchWord>& words : query.excludes)
		{
			if (MatchesSearchTerm(words, keywords))
			{
				return false;
			}
		}
		if (query.includes.IsEmpty())
		{
			return true;
		}
		for (const TArray<SearchWord>& words : query.includes)
		{
			if (MatchesSearchTerm(words, keywords))
			{
				return true;
			}
		}
		return false;
	}

	// Scores of the different ways a search word can match a name. Used to rank results
	constexpr i32 searchExactScore    = 400;
	constexpr i32 searchPrefixScore   = 300;
	constexpr i32 searchContainsScore = 200;
	// Matches of secondary names rank below matches of the item's own name
	constexpr i32 searchKeywordPenalty = 150;

	// How well a word matches a name. Exact matches score the highest, followed by
	// prefixes and substrings
	static i32 ScoreSearchText(StringView text, StringView word)
	{
		if (word.empty())
		{
			return 0;
		}
		if (p::Strings::Equals(text, word))
		{
			return searchExactScore;
		}
		const sizet found = p::Strings::Find(text, word);
		if (found == 0)
		{
			return searchPrefixScore;
		}
		if (found != StringView::npos)
		{
			return searchContainsScore - i32(p::Min(found, sizet(99)));
		}
		return 0;
	}

	// How well a search word matches a name, on its plain or standardized form
	static i32 ScoreSearchWord(const SearchKeyword& keyword, const SearchWord& word)
	{
		i32 score = ScoreSearchText(keyword.name, word.name);
		if (!word.standardized.empty() && !keyword.standardized.empty())
		{
			score = p::Max(score, ScoreSearchText(keyword.standardized, word.standardized));
		}
		return score;
	}

	// The best score of an entry against the search, used to rank results. A term scores
	// the sum of its words, and matches of the item's own name rank above matches of its
	// other names
	static i32 ScoreSearch(const SearchQuery& query, const TArray<SearchKeyword>& keywords)
	{
		i32 best = 0;
		for (const TArray<SearchWord>& words : query.includes)
		{
			i32 termScore = 0;
			for (const SearchWord& word : words)
			{
				i32 wordScore = 0;
				for (i32 keywordIdx = 0; keywordIdx < keywords.Size(); ++keywordIdx)
				{
					const i32 penalty = keywordIdx > 0 ? searchKeywordPenalty : 0;
					wordScore =
					    p::Max(wordScore, ScoreSearchWord(keywords[keywordIdx], word) - penalty);
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
					// Input matches type?
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
					// Output matches type?
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

	bool ContextItem(StringView name, Id id, Color background = Color::Transparent())
	{
		if (id != p::NoId)
		{
			UI::PushID(id);
		}
		constexpr float rounding = 2.f;

		// Keep the click area tight to the item so the 1px spacing between items shows
		ImGuiSelectableFlags flags = ImGuiSelectableFlags_NoPadWithHalfSpacing;
		const bool hasBackground   = background.a > 0.f;
		if (hasBackground)
		{
			UI::PushHeaderColor(background);
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

	constexpr float contextChipRounding = 2.f;
	constexpr float contextChipPadding  = 4.f;
	constexpr float contextChipSpacing  = 4.f;

	// Draws a context menu row. When a background is given it's painted by the row itself,
	// with the content drawn on top. Returns whether the row was picked
	bool DrawContextChipRow(StringView label, ImVec2& outMin, ImVec2& outMax,
	    Color background = Color::Transparent(), float height = 16.f)
	{
		UI::PushStyleVar(ImGuiStyleVar_ItemSpacing, v2{UI::GetStyle().ItemSpacing.x, 2.f});
		UI::PushStyleVar(ImGuiStyleVar_SelectableRounding, contextChipRounding);
		const bool hasBackground = background.a > 0.f;
		if (hasBackground)
		{
			UI::PushHeaderColor(background);
		}
		const bool pressed = UI::Selectable(
		    label, hasBackground, ImGuiSelectableFlags_NoPadWithHalfSpacing, v2{0.f, height});
		if (hasBackground)
		{
			UI::PopHeaderColor();
		}
		UI::PopStyleVar(2);
		if (pressed)
		{
			UI::CloseCurrentPopup();
		}

		outMin = UI::GetItemRectMin();
		outMax = UI::GetItemRectMax();
		return pressed;
	}

	// Draws text centered on the row, advancing 'x'
	void DrawContextItemText(ImDrawList& drawList, float& x, float centerY, StringView text)
	{
		const ImVec2 size = UI::CalcTextSize(text);
		drawList.AddText(ImVec2{x, centerY - size.y * 0.5f}, UI::GetColorU32(ImGuiCol_Text),
		    text.data(), text.data() + text.size());
		x += size.x;
	}

	// Draws a type name over a rounded background of its own color, advancing 'x'. The icon,
	// when given, is drawn smaller and before the name
	void DrawContextTypeChip(ImDrawList& drawList, float& x, float centerY, StringView name,
	    const Color& color, const char* icon = nullptr)
	{
		ImFont* font          = UI::GetFont();
		const float fontSize  = UI::GetFontSize();
		const float iconSize  = fontSize * 0.75f;
		const float iconWidth = icon ? font->CalcTextSizeA(iconSize, FLT_MAX, 0.f, icon).x : 0.f;

		const ImVec2 textSize    = UI::CalcTextSize(name);
		const float contentWidth = iconWidth + textSize.x;
		const float height       = UI::GetTextLineHeight() + 2.f;
		const ImVec2 chipMin{x, centerY - height * 0.5f};
		const ImVec2 chipMax{x + contentWidth + contextChipPadding * 2.f, centerY + height * 0.5f};

		drawList.AddRectFilled(chipMin, chipMax, color.ToPackedABGR(), contextChipRounding);
		// The icon shares the top of the text line, instead of being centred on its own
		if (icon)
		{
			const float textY = centerY - (iconSize * 0.5f) + 2.f;
			drawList.AddText(font, iconSize, ImVec2{chipMin.x + contextChipPadding, textY},
			    UI::GetColorU32(ImGuiCol_Text), icon);
		}
		const float textY = centerY - textSize.y * 0.5f;
		drawList.AddText(ImVec2{chipMin.x + contextChipPadding + iconWidth, textY},
		    UI::GetColorU32(ImGuiCol_Text), name.data(), name.data() + name.size());
		x = chipMax.x;
	}

	// A cast of the context menu: each type carries a rounded background of its own color
	bool ContextCastItem(ast::Tree& ast, const ast::Cast& cast)
	{
		ImDrawList* drawList      = UI::GetWindowDrawList();
		const StringView fromName = ast::GetName(ast, cast.fromType).AsString();
		const StringView toName   = ast::GetName(ast, cast.toType).AsString();

		// Both types identify the cast in the menu
		UI::PushID(cast.fromType);
		UI::PushID(cast.toType);
		ImVec2 itemMin;
		ImVec2 itemMax;
		const bool pressed  = DrawContextChipRow("###Item", itemMin, itemMax);
		const float centerY = (itemMin.y + itemMax.y) * 0.5f;
		float x             = itemMin.x + contextChipPadding;
		DrawContextTypeChip(*drawList, x, centerY, fromName, GetTypeColor(ast, cast.fromType));
		x += contextChipSpacing;
		DrawContextItemText(*drawList, x, centerY, ICON_FA_ARROW_RIGHT);
		x += contextChipSpacing;
		DrawContextTypeChip(*drawList, x, centerY, toName, GetTypeColor(ast, cast.toType));
		UI::PopID();
		UI::PopID();
		return pressed;
	}

	// A function of the context menu: its name followed by its arguments and returns, each a
	// chip with its name colored as its type. Returns are marked with an upward arrow
	bool ContextFunctionItem(ast::Tree& ast, Id id, StringView name)
	{
		ImDrawList* drawList = UI::GetWindowDrawList();
		UI::PushID(id);
		ImVec2 itemMin;
		ImVec2 itemMax;

		// The function carries its own background, slightly thicker than the pin chips
		const float rowHeight = UI::GetTextLineHeight() + 4.f;
		const bool pressed =
		    DrawContextChipRow("###Item", itemMin, itemMax, functionColor, rowHeight);
		const float centerY = (itemMin.y + itemMax.y) * 0.5f;
		float x             = itemMin.x + contextChipPadding;

		// Arguments produce a value inside the function, returns consume one
		const auto* args    = ast.TryGet<const ast::CExprOutputs>(id);
		const auto* returns = ast.TryGet<const ast::CExprInputs>(id);

		bool first   = true;
		auto drawPin = [&](Id pinId, bool isReturn)
		{
			DrawContextItemText(*drawList, x, centerY, first ? "" : ", ");
			first = false;

			// The chip carries the name of the pin, colored as its type. Returns lead with an
			// arrow inside of the chip
			const auto* ns = ast.TryGet<const ast::CNamespace>(pinId);
			DrawContextTypeChip(*drawList, x, centerY,
			    ns ? ns->name.AsString() : StringView{"none"},
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

		DrawContextItemText(*drawList, x, centerY, name);
		DrawContextItemText(*drawList, x, centerY, "(");
		if (args)
		{
			drawPins(args->pinIds, false);
		}
		if (returns)
		{
			drawPins(returns->pinIds, true);
		}
		DrawContextItemText(*drawList, x, centerY, ")");
		UI::PopID();
		return pressed;
	}

	// A constructor of the context menu: the type carries a rounded background of its color
	bool ContextConstructItem(StringView typeName, Id id, const Color& color)
	{
		ImDrawList* drawList = UI::GetWindowDrawList();
		UI::PushID(id);
		ImVec2 itemMin;
		ImVec2 itemMax;
		const bool pressed  = DrawContextChipRow("###Item", itemMin, itemMax);
		const float centerY = (itemMin.y + itemMax.y) * 0.5f;
		float x             = itemMin.x + contextChipPadding;
		DrawContextItemText(*drawList, x, centerY, "Make ");
		x += contextChipSpacing;
		DrawContextTypeChip(*drawList, x, centerY, typeName, color);
		UI::PopID();
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
		if (nodeIds.Size() == 1 && ast.Has<ast::CDeclFunction>(firstNodeId))
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

	// Caches the functions and variables under a namespace: functions and variables become
	// entries, modules and types become categories
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
				entry.label = String{childName};
				entry.AddKeyword(String{childName});
				cache.AddEntry(p::Move(entry), parent);
			}
			else if (ast.Has<ast::CDeclVariable>(childId))
			{
				if (!FitsContext(ast, context, ContextItemKind::Variable, childId))
				{
					continue;
				}
				const auto* variable = ast.TryGet<const ast::CDeclVariable>(childId);
				MenuEntry entry;
				entry.kind       = ContextItemKind::Variable;
				entry.id         = childId;
				entry.label      = String{childName};
				entry.background = GetTypeColor(ast, variable ? variable->typeId : ast::NoId);
				entry.AddKeyword(String{childName});
				cache.AddEntry(p::Move(entry), parent);
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

				const StringView icon = GetFileIcon(ast, childId);
				const String label    = p::Format("{} {}", icon, childName);
				const i32 category    = cache.AddCategory(label, parent);
				AddNamespaceEntries(cache, ast, context, categoryChildren, category);
			}
		}
	}

	// Caches the flow, operator, constructor and cast entries of the menu
	static void AddBuiltinEntries(MenuCache& cache, ast::Tree& ast, const MenuContext& context)
	{
		const i32 flow = cache.AddCategory("Flow", 0);
		if (FitsContext(ast, context, ContextItemKind::Return))
		{
			MenuEntry entry;
			entry.kind  = ContextItemKind::Return;
			entry.label = "Return";
			entry.AddKeyword("Return");
			cache.AddEntry(p::Move(entry), flow);
		}
		if (FitsContext(ast, context, ContextItemKind::If))
		{
			MenuEntry entry;
			entry.kind  = ContextItemKind::If;
			entry.label = "If";
			entry.AddKeyword("If");
			cache.AddEntry(p::Move(entry), flow);
		}

		const i32 operators = cache.AddCategory("Operators", 0);
		String label;
		// Unary operators
		for (auto type : GetEnumValues<ast::UnaryOperatorType>())
		{
			if (!FitsContext(ast, context, ContextItemKind::UnaryOperator))
			{
				continue;
			}
			const StringView shortName = GetUnaryOperatorName(type);
			const StringView longName  = GetUnaryOperatorLongName(type);
			label.clear();
			p::FormatTo(label, "{}   ({})", shortName, longName);
			MenuEntry entry;
			entry.kind         = ContextItemKind::UnaryOperator;
			entry.operatorType = i32(type);
			entry.label        = label;
			entry.AddKeyword(String{shortName});
			entry.AddKeyword(String{longName});
			cache.AddEntry(p::Move(entry), operators);
		}
		// Binary operators
		for (auto type : GetEnumValues<ast::BinaryOperatorType>())
		{
			if (!FitsContext(ast, context, ContextItemKind::BinaryOperator))
			{
				continue;
			}
			const StringView shortName = GetBinaryOperatorName(type);
			const StringView longName  = GetBinaryOperatorLongName(type);
			label.clear();
			p::FormatTo(label, "{}   ({})", shortName, longName);
			MenuEntry entry;
			entry.kind         = ContextItemKind::BinaryOperator;
			entry.operatorType = i32(type);
			entry.label        = label;
			entry.AddKeyword(String{shortName});
			entry.AddKeyword(String{longName});
			cache.AddEntry(p::Move(entry), operators);
		}

		const i32 constructors = cache.AddCategory("Constructors", 0);
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
				entry.label      = String{typeName};
				entry.background = GetTypeColor(ast, literalTypeId);
				entry.AddKeyword("Make");
				entry.AddKeyword(String{typeName});
				cache.AddEntry(p::Move(entry), constructors);
			}
		}

		const i32 casts = cache.AddCategory("Casts", 0);
		for (const ast::Cast& cast : ast::CastSystem::GetCasts(ast))
		{
			if (!FitsContext(ast, context, ContextItemKind::Cast, ast::NoId, &cast))
			{
				continue;
			}
			MenuEntry entry;
			entry.kind = ContextItemKind::Cast;
			entry.cast = cast;
			entry.AddKeyword("Cast");
			entry.AddKeyword(String{ast::GetName(ast, cast.fromType).AsString()});
			entry.AddKeyword(String{ast::GetName(ast, cast.toType).AsString()});
			cache.AddEntry(p::Move(entry), casts);
		}
	}

	// Caches every entry of the menu before drawing, so each frame only filters and scores
	// the cached entries
	static void BuildMenuCache(MenuCache& cache, ast::Tree& ast, const MenuContext& context)
	{
		cache.Reset();
		AddBuiltinEntries(cache, ast, context);
		// Functions and variables, grouped under their module and type namespaces
		AddNamespaceEntries(
		    cache, ast, context, p::FindAllIdsWith<ast::CModule, ast::CNamespace>(ast), 0);
	}

	// Collects the visible entries of a menu category in order. Categories themselves are
	// ignored
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

	// Updates which cached entries match the search and how well they match, along with
	// the draw order of every category. A category scores as its best entry
	static void FilterMenuCache(MenuCache& cache, const ImGuiTextFilter& filter)
	{
		const bool isSearching  = filter.IsActive();
		const SearchQuery query = ParseSearch(filter.InputBuf);

		for (MenuEntry& entry : cache.entries)
		{
			entry.isVisible = !isSearching || MatchesSearch(query, entry.keywords);
			entry.score     = entry.isVisible ? ScoreSearch(query, entry.keywords) : 0;
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
	static void DrawMenuEntry(
	    ast::Tree& ast, Id typeId, const MenuContext& context, const MenuEntry& entry)
	{
		bool pressed = false;
		switch (entry.kind)
		{
			case ContextItemKind::Function:
				pressed = ContextFunctionItem(ast, entry.id, entry.label.AsView());
				break;
			case ContextItemKind::Literal:
				pressed = ContextConstructItem(entry.label.AsView(), entry.id, entry.background);
				break;
			case ContextItemKind::Cast: pressed = ContextCastItem(ast, entry.cast); break;
			case ContextItemKind::Return:
			case ContextItemKind::If:
			case ContextItemKind::Variable:
			case ContextItemKind::UnaryOperator:
			case ContextItemKind::BinaryOperator:
				pressed = ContextItem(entry.label.AsView(), entry.id, entry.background);
				break;
		}
		if (pressed)
		{
			SpawnEntryNode(ast, typeId, context, entry);
		}
	}

	// Marks the selected item: highlights it and scrolls it into view
	static void DrawSelectedItem()
	{
		UI::GetWindowDrawList()->AddRect(UI::GetItemRectMin(), UI::GetItemRectMax(),
		    UI::GetColorU32(ImGuiCol_NavCursor), 2.f, 0, 2.f);

		// The results are drawn in a child that doesn't scroll, so scroll the popup
		ImGuiWindow* window = UI::GetCurrentWindow();
		if (window->ParentWindow)
		{
			UI::ScrollToBringRectIntoView(
			    window->ParentWindow, ImRect(UI::GetItemRectMin(), UI::GetItemRectMax()));
		}
	}

	static void DrawMenuCategory(ast::Tree& ast, Id typeId, const MenuContext& context,
	    MenuCache& cache, i32 categoryIdx, bool showSelection);

	// Draws the entries and subcategories of a category. The first item drawn becomes the
	// selected one when the results change
	static void DrawMenuCategoryContents(ast::Tree& ast, Id typeId, const MenuContext& context,
	    MenuCache& cache, const MenuCategory& category, bool showSelection)
	{
		for (const i32 entryIdx : category.visibleEntries)
		{
			DrawMenuEntry(ast, typeId, context, cache.entries[entryIdx]);
			if (cache.selectedEntry == -1)
			{
				cache.selectedEntry = entryIdx;
			}
			if (showSelection && entryIdx == cache.selectedEntry)
			{
				DrawSelectedItem();
			}
		}
		for (const i32 childIdx : category.visibleChildren)
		{
			DrawMenuCategory(ast, typeId, context, cache, childIdx, showSelection);
		}
	}

	// Draws a cached category and its contents. Categories with nothing visible inside are
	// skipped, and they draw open by default so the first item is always reachable
	static void DrawMenuCategory(ast::Tree& ast, Id typeId, const MenuContext& context,
	    MenuCache& cache, i32 categoryIdx, bool showSelection)
	{
		const MenuCategory& category = cache.categories[categoryIdx];
		if (category.visibleEntries.IsEmpty() && category.visibleChildren.IsEmpty())
		{
			return;
		}

		// Categories open by default, so the first item is selected even without a search.
		// Only applies while the user hasn't opened or closed the category themselves
		UI::SetNextItemOpen(true, ImGuiCond_Once);
		if (!UI::TreeNode(category.label.c_str()))
		{
			return;
		}
		DrawMenuCategoryContents(ast, typeId, context, cache, category, showSelection);
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
		if (filterActive)
		{
			// Up and down move the selected item below instead of navigating the window,
			// so the search can keep being refined while browsing results
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

		// The default item takes the focus while the results change, so Enter applies it
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
			if (focusDefault)
			{
				cache.selectedEntry = -1;
			}

			// While the search input is active, up and down move the selected item
			if (filterActive && !cache.flatEntries.IsEmpty())
			{
				i32 selectionIdx = cache.flatEntries.FindIndex(cache.selectedEntry);
				if (UI::IsKeyPressed(ImGuiKey_DownArrow))
				{
					++selectionIdx;
				}
				if (UI::IsKeyPressed(ImGuiKey_UpArrow))
				{
					--selectionIdx;
				}
				selectionIdx        = p::Clamp(selectionIdx, 0, cache.flatEntries.Size() - 1);
				cache.selectedEntry = cache.flatEntries[selectionIdx];
			}

			// The hidden root category only wraps the contents of the menu
			DrawMenuCategoryContents(
			    ast, typeId, context, cache, cache.categories[0], filterActive);

			// Enter applies the selected item, or the default one when nothing is selected
			if (filterActive && UI::IsKeyPressed(ImGuiKey_Enter))
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
