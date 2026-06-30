// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "UI/UIImgui.h"

#include <Pipe/Core/String.h>


namespace rift::UI
{
	enum class ToastType : p::u8
	{
		None = 0,
		Success,
		Warning,
		Error,
		Info
	};

	struct Toast
	{
		ToastType type     = ToastType::None;
		float durationTime = 3.f;
		p::String title;
		p::String message;
	};

	void AddNotification(Toast toast);

	void DrawNotifications();
}    // namespace rift::UI
