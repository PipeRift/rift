// Copyright 2015-2021 Piperift - All rights reserved

#include "Assets/TypeAsset.h"

#include <Lang/AST/ASTArchive.h>


namespace Rift
{
	void TypeAsset::Serialize(Archive& ar, StringView name)
	{
		ar.BeginObject(name);

		u8 typeValue = ar.IsLoading() ? 0 : u8(type);
		ar("type", typeValue);
		type = Type(typeValue);

		SerializeReflection(ar);

		ASTArchive astArchive{ar};
		// astArchive("declaration", declaration);

		ar.EndObject();
	}
}    // namespace Rift
