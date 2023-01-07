// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "UI/UIImgui.h"

#include <Pipe/Core/String.h>


namespace rift::UI
{
	enum class ToastType : u8
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
		String title;
		String message;
	};

	void AddNotification(Toast toast);

	void DrawNotifications();
}    // namespace rift::UI
