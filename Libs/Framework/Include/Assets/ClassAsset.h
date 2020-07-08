// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Lenguage/Function.h"
#include "TypeAsset.h"

#include <Assets/AssetPtr.h>
#include <CoreObject.h>


class ClassAsset : public TypeAsset
{
	CLASS(ClassAsset, TypeAsset)

	P(Name, name);

	P(TArray<Function>, functions);
};
