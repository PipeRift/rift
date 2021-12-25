// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "UI/UIImGui.h"

#include <Strings/String.h>


namespace Rift::UI
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
}    // namespace Rift::UI
