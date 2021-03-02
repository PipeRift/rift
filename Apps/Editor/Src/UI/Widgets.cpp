// Copyright 2015-2021 Piperift - All rights reserved

#include "UI/Widgets.h"


namespace Rift::UI
{
	void AnimatedSprite::SetAnimation(u32 id)
	{
		currentFrame              = {0, Math::Clamp(id, 0u, u32(numFrames.Size() - 1))};
		currentFrameRemainingTime = rate;
	}

	void AnimatedSprite::NextFrame(float deltaTime)
	{
		currentFrameRemainingTime -= deltaTime;

		if (currentFrameRemainingTime <= 0.f)
		{
			++currentFrame.x;
			if (currentFrame.x >= numFrames[currentFrame.y])
			{
				currentFrame.x = 0;
			}
			currentFrameRemainingTime = rate;
		}
	}

	v2 AnimatedSprite::GetUV() const
	{
		return size * currentFrame;
	}

	bool SpriteButton(AnimatedSprite& sprite, i32 framePadding, const LinearColor& bgColor,
	    const LinearColor& tintColor)
	{
		const v2 uv = sprite.GetUV();
		UI::ImageButton(sprite.textureId, sprite.size, uv, v2(uv + sprite.size), framePadding,
		    bgColor, tintColor);
	}
}    // namespace Rift::UI
