// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"
#include "Assets/ModuleAsset.h"

#include <CoreObject.h>


namespace Rift
{
	struct CEntityLoadUnique : public Struct
	{
		STRUCT(CEntityLoadUnique, Struct)

		struct FileText
		{
			AST::Id id;
			String data;
		};

		TArray<AST::Id> entitiesToLoadSync;
		TArray<AST::Id> entitiesToLoadAsync;

		TArray<AST::Id> filesToLoad;
		TArray<FileText> loadedFiles;
	};
}    // namespace Rift
