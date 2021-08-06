// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"
#include "Assets/ModuleAsset.h"

#include <CoreObject.h>


namespace Rift
{
	// Contains loaded string data from disk
	struct CStringLoadUnique : public Struct
	{
		STRUCT(CStringLoadUnique, Struct)

		struct FileText
		{
			AST::Id id;
			String data;
		};

		TArray<AST::Id> filesToLoad;
		TArray<FileText> loadedFiles;
	};
}    // namespace Rift
