// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include <PipeContainers.h>
#include <PipeReflect.h>


namespace rift::editor
{
	struct ZoomSettings
	{
		P_STRUCT(ZoomSettings)

		P_PROP(min, p::PF_Edit)
		float min = 0.25f;

		P_PROP(max, p::PF_Edit)
		float max = 4.f;

		P_PROP(defaultZoom, p::PF_Edit)
		float defaultZoom = 1.f;

		// Multiplier applied to the zoom for every mouse-wheel notch.
		P_PROP(step, p::PF_Edit)
		float step = 1.1f;

		// Last zoom used. It is persisted so it can be restored, but it is not meant to be
		// edited directly, hence it has no PF_Edit flag.
		P_PROP(current)
		float current = 1.f;
	};

	struct EditorSettings
	{
		P_STRUCT(EditorSettings)

		P_PROP(recentProjects)
		p::TArray<p::String> recentProjects;

		P_PROP(gridSize, p::PF_Edit)
		float gridSize = 16.f;

		P_PROP(zoom, p::PF_Edit)
		ZoomSettings zoom;
	};
}    // namespace rift::editor
