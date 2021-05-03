// Copyright 2015-2021 Piperift - All rights reserved

#include "Assets/TypeAsset.h"

#include <Lang/AST/ASTArchive.h>


namespace Rift
{
	void TypeAsset::Serialize(Archive& ar, StringView name)
	{
		ar.BeginObject(name);
		SerializeReflection(ar);

		ASTArchive astArchive{ar};
		// astArchive("declaration", declaration);

		ar.EndObject();
	}
}    // namespace Rift
