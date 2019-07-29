// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include "CoreObject.h"

#include <EASTL/vector.h>


class Factory {
public:
	Class* SupportedClass;


	Factory() = default;
	virtual ~Factory() = default;


	static GlobalPtr<Object> CreateNewAsset(Ptr<Object> Owner, Name Name) {
		return {};
	}

	/*virtual Color GetThumbnailColor(Ptr<T> Instance) {
		return Color::Cyan;
	}*/


	static eastl::vector<eastl::unique_ptr<Factory>> Factories;
};
