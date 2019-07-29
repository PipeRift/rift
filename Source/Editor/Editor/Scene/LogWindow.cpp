// Copyright 2015-2019 Piperift - All rights reserved

#include "LogWindow.h"
#include "Core/Log.h"


#if WITH_EDITOR

Ptr<LogWindow> LogWindow::globalLogWindow {};


void LogWindow::Log(const String& str)
{
	textBuffer.append(str.c_str(), str.end());
	textBuffer.append("\n");

	lineOffsets.Add(lineOffsets.Last() + i32(str.size()) + 1);

	if (bAutoScroll)
	{
		bScrollToBottom = true;
	}
}

void LogWindow::Build()
{
	Super::Build();
	SetName(TX("Log"));

	Clear();
	globalLogWindow = Self<LogWindow>();
}

void LogWindow::Tick(float)
{
	if (bOpen)
	{
		BeginWindow();

		// Options menu
		if (ImGui::BeginPopup("Options"))
		{
			if (ImGui::Checkbox("Auto-scroll", &bAutoScroll) && bAutoScroll)
			{
				bScrollToBottom = true;
			}
			ImGui::EndPopup();
		}

		// Main Window
		if (ImGui::Button("Options"))
		{
			ImGui::OpenPopup("Options");
		}
		ImGui::SameLine();
		const bool bClear = ImGui::Button("Clear");
		ImGui::SameLine();
		const bool bCopy = ImGui::Button("Copy");
		ImGui::SameLine();
		filter.Draw("Filter", -100.0f);

		ImGui::Separator();
		ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

		if (bClear)
		{
			Clear();
		}
		if (bCopy)
		{
			ImGui::LogToClipboard();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		const char* buf = textBuffer.begin();
		const char* buf_end = textBuffer.end();
		if (filter.IsActive())
		{
			// In this example we don't use the clipper when Filter is enabled.
			// This is because we don't have a random access on the result on our filter.
			// A real application processing logs with ten of thousands of entries may want to store the result of search/filter.
			// especially if the filtering function is not trivial (e.g. reg-exp).
			for (i32 line_no = 0; line_no < lineOffsets.Size(); ++line_no)
			{
				const char* line_start = buf + lineOffsets[line_no];
				const char* line_end = (line_no + 1 < lineOffsets.Size()) ? (buf + lineOffsets[line_no + 1] - 1) : buf_end;
				if (filter.PassFilter(line_start, line_end))
					ImGui::TextUnformatted(line_start, line_end);
			}
		}
		else
		{
			// The simplest and easy way to display the entire buffer:
			//   ImGui::TextUnformatted(buf_begin, buf_end);
			// And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward to skip non-visible lines.
			// Here we instead demonstrate using the clipper to only process lines that are within the visible area.
			// If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them on your side is recommended.
			// Using ImGuiListClipper requires A) random access into your data, and B) items all being the  same height,
			// both of which we can handle since we an array pointing to the beginning of each line of text.
			// When using the filter (in the block of code above) we don't have random access into the data to display anymore, which is why we don't use the clipper.
			// Storing or skimming through the search result would make it possible (and would be recommended if you want to search through tens of thousands of entries)
			ImGuiListClipper clipper;
			clipper.Begin(lineOffsets.Size());
			while (clipper.Step())
			{
				for (i32 line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; ++line_no)
				{
					const char* line_start = buf + lineOffsets[line_no];
					const char* line_end = (line_no + 1 < lineOffsets.Size()) ? (buf + lineOffsets[line_no + 1] - 1) : buf_end;
					ImGui::TextUnformatted(line_start, line_end);
				}
			}
			clipper.End();
		}
		ImGui::PopStyleVar();

		if (bScrollToBottom)
		{
			ImGui::SetScrollHereY(1.0f);
		}
		bScrollToBottom = false;

		ImGui::Spacing();
		ImGui::EndChild();
		EndWindow();
	}
}

#endif
