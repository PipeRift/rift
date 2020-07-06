// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "TypeAsset.h"

#include <Assets/AssetPtr.h>
#include <CoreObject.h>
#include "Lenguaje/Function.h"


class ClassAsset : public TypeAsset
{
	CLASS(ClassAsset, TypeAsset)

	P(Name, name);

	P(TArray<Function>, functions);
};
