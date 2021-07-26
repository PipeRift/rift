// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <Assets/AssetInfo.h>
#include <Misc/Guid.h>
#include <Object/Struct.h>


namespace Rift
{
	struct CType : public Struct
	{
		STRUCT(CType, Struct)

		PROP(Path, path)
		Path path;

		PROP(AST::Id, moduleId)
		AST::Id moduleId = AST::NoId;

		PROP(Guid, guid)
		Guid guid;


		CType() {}
	};
}    // namespace Rift
