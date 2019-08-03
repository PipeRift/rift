// © 2019 Miguel Fernández Arce - All rights reserved

#pragma once

#include "CoreObject.h"
#include "Core/Assets/AssetPtr.h"

#include "BaseAsset.h"
#include "ClassAsset.h"
#include "StructAsset.h"
#include "Graph/FunctionIdentifier.h"


class LibraryAsset : public BaseAsset
{
	CLASS(LibraryAsset, BaseAsset)

	TArray<TAssetPtr<ClassAsset>> classes;
	TArray<TAssetPtr<StructAsset>> structs;

	TArray<FunctionIdentifier> functions;
};
