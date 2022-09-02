// Copyright 2015-2022 Piperift - All rights reserved

#include "Utils/EditorStyle.h"

#include "AST/Components/CDeclClass.h"
#include "AST/Components/CDeclStruct.h"

#include <UI/Style.h>
#include <Utils/Nodes.h>


namespace rift::Editor
{
	const Color GetTypeColor(const AST::Tree& ast, AST::Id id)
	{
		if (!ast.IsValid(id))
		{
			return GetTypeColor<void>();
		}

		const auto& nativeIds = ast.GetNativeTypes();
		if (id == nativeIds.boolId)
		{
			return GetTypeColor<bool>();
		}
		// Floating point
		else if (id == nativeIds.floatId || id == nativeIds.doubleId)
		{
			return GetTypeColor<double>();
		}
		// Unsigned integer
		else if (id == nativeIds.u8Id || id == nativeIds.u16Id || id == nativeIds.u32Id
		         || id == nativeIds.u64Id)
		{
			return GetTypeColor<u32>();
		}
		// Signed integer
		else if (id == nativeIds.i8Id || id == nativeIds.i16Id || id == nativeIds.i32Id
		         || id == nativeIds.i64Id)
		{
			return GetTypeColor<i32>();
		}
		else if (id == nativeIds.stringId)
		{
			return GetTypeColor<String>();
		}
		else if (ast.Has<AST::CDeclClass>(id))
		{
			return GetTypeColor<Class>();
		}
		else if (ast.Has<AST::CDeclStruct>(id))
		{
			return GetTypeColor<Struct>();
		}
		return GetTypeColor<void>();
	}


	void PushNodeTitleColor(Color color)
	{
		Nodes::PushStyleColor(Nodes::ColorVar_TitleBar, color);
		Nodes::PushStyleColor(Nodes::ColorVar_TitleBarHovered, UI::Hovered(color));
		Nodes::PushStyleColor(Nodes::ColorVar_TitleBarSelected, color);
	}

	void PopNodeTitleColor()
	{
		Nodes::PopStyleColor(3);
	}

	void PushNodeBackgroundColor(Color color)
	{
		Nodes::PushStyleColor(Nodes::ColorVar_NodeBackground, color);
		Nodes::PushStyleColor(Nodes::ColorVar_NodeBackgroundHovered, UI::Hovered(color));
		Nodes::PushStyleColor(Nodes::ColorVar_NodeBackgroundSelected, color);
	}

	void PopNodeBackgroundColor()
	{
		Nodes::PopStyleColor(3);
	}
}    // namespace rift::Editor
