// Copyright 2015-2020 Piperift - All rights reserved

#include "Misc/Time.h"

#include "Profiler.h"

#include <thread>


namespace VCLang
{
	void FrameTime::Tick()
	{
		previousTime = currentTime;
		currentTime = DateTime::UtcNow();

		// Avoid too big delta times
		realDeltaTime = Math::Min(0.15f, (currentTime - previousTime).GetTotalSeconds());

		// Apply time dilation
		deltaTime = realDeltaTime * timeDilation;
	}

	void FrameTime::PostTick()
	{
		float extraTimeForFPSCAP =
			minFrameTime - (DateTime::UtcNow() - currentTime).GetTotalSeconds();
		if (extraTimeForFPSCAP > 0.0f)
		{
			// Cap FPS with a delay
			ScopedZone("Sleep", 0xD15545);

			std::chrono::duration<float, std::chrono::seconds::period> sleepPeriod{
				extraTimeForFPSCAP};
			std::this_thread::sleep_for(sleepPeriod);
		}
	}
}	 // namespace VCLang
