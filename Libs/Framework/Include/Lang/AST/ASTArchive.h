// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Serialization/NestedArchive.h"


namespace Rift
{
	class ASTArchive : public NestedArchive
	{
	public:
		ASTArchive(Archive& inParent) : NestedArchive(inParent) {}

	private:
		void Serialize(StringView name, AST::Id& val)
		{
			Log::Info("ID");
		}
	};
}    // namespace Rift
