// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "CoreTypes.h"
#include "DateTime.h"


namespace VCLang
{
	struct CORE_API FrameTime
	{
	protected:
		DateTime previousTime;
		DateTime currentTime;

		float realDeltaTime;
		float deltaTime;

		float timeDilation;

		// Value of 1/FPS_CAP
		float minFrameTime;

	public:
		FrameTime()
			: previousTime{}
			, currentTime{DateTime::UtcNow()}
			, realDeltaTime(0.f)
			, deltaTime(0.f)
			, timeDilation(1.f)
		{}

		void Tick();

		void PostTick();

		void SetFPSCap(u32 maxFPS)
		{
			minFrameTime = 1.f / maxFPS;
		}

		void SetTimeDilation(float newTimeDilation)
		{
			timeDilation = newTimeDilation;
		}

		float GetDeltaTime() const
		{
			return deltaTime;
		}
	};
}	 // namespace VCLang
