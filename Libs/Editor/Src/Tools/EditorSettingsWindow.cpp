// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Tools/EditorSettingsWindow.h"

#include "Statics/EditorSettings.h"
#include "Utils/FunctionGraph.h"

#include <AST/Utils/Settings.h>
#include <UI/Inspection.h>


namespace rift::editor
{
	void EditorSettingsWindow::Draw()
	{
		if (!open)
		{
			return;
		}

		UI::SetNextWindowSize(p::v2{360.f, 280.f}, ImGuiCond_FirstUseEver);
		if (UI::Begin("Editor Settings", &open))
		{
			EditorSettings& settings = GetUserSettings<EditorSettings>();

			// Snapshot the editable values to detect and react to inspector changes.
			const float gridSizeBefore    = settings.gridSize;
			const ZoomSettings zoomBefore = settings.zoom;

			if (UI::BeginInspector("EditorSettingsInspector", p::v2{0.f, 0.f}, true))
			{
				// Only editable properties are shown: the persisted current zoom and the
				// recent projects list are intentionally left out.
				UI::InspectStruct(&settings);
				UI::EndInspector();
			}

			if (settings.gridSize != gridSizeBefore || settings.zoom.min != zoomBefore.min
			    || settings.zoom.max != zoomBefore.max
			    || settings.zoom.defaultZoom != zoomBefore.defaultZoom
			    || settings.zoom.step != zoomBefore.step)
			{
				// Keep the values sane and push them to the node editor.
				settings.gridSize = p::Clamp(settings.gridSize, 4.f, 64.f);
				settings.zoom.min = p::Clamp(settings.zoom.min, 0.05f, 0.95f);
				settings.zoom.max = p::Clamp(settings.zoom.max, 1.05f, 10.f);
				settings.zoom.defaultZoom =
				    p::Clamp(settings.zoom.defaultZoom, settings.zoom.min, settings.zoom.max);
				settings.zoom.step = p::Max(settings.zoom.step, 1.01f);

				Graph::ApplyGraphSettings();
				SaveUserSettings<EditorSettings>();
			}
		}
		UI::End();
	}
}    // namespace rift::editor
