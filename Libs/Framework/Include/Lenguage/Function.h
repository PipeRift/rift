// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Assets/TypeAsset.h"

#include <Assets/AssetPtr.h>
#include <CoreEngine.h>
#include <Object/Struct.h>


namespace VCLang
{
	struct FunctionArgument : public Struct
	{
		STRUCT(FunctionArgument, Struct)

		PROP(Name, name);
		Name name;

		PROP(TAssetPtr<TypeAsset>, type);
		TAssetPtr<TypeAsset> type;
	};

	struct Function : public Struct
	{
		STRUCT(Function, Struct)

		PROP(Name, name);
		Name name;

		PROP(TArray<FunctionArgument>, arguments);
		TArray<FunctionArgument> arguments;

		PROP(TAssetPtr<TypeAsset>, returnType);
		TAssetPtr<TypeAsset> returnType;
	};
}	 // namespace VCLang
