// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreStruct.h"
#include "Core/Assets/AssetData.h"

#include "Container.h"
#include "VariableIdentifier.h"
#include "FunctionIdentifier.h"


class ClassAsset : public AssetData
{
	STRUCT(ClassAsset, AssetData)

	P(TArray<VariableIdentifier>, variables);

	P(TArray<FunctionIdentifier>, functions);
};
