// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <CoreMinimal.h>


namespace Rift
{
	class AssetSystem : public Object
	{
		CLASS(AssetSystem, Object)

	public:
		void Init();
		void Tick();
	};
}    // namespace Rift
