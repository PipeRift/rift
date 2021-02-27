
#include "Style.h"

#include "Misc/Imgui.h"

#include <Math/Color.h>


using namespace Rift;

namespace Style
{
	void ApplyStyle()
	{
		ImGui::StyleColorsDark();
		auto& style = ImGui::GetStyle();

		ImVec4* colors                      = style.Colors;
		colors[ImGuiCol_FrameBg]            = Color(0.48f, 0.40f, 0.16f, 0.54f);
		colors[ImGuiCol_FrameBgHovered]     = ImVec4(0.98f, 0.77f, 0.26f, 0.40f);
		colors[ImGuiCol_FrameBgActive]      = ImVec4(0.98f, 0.77f, 0.26f, 0.67f);
		colors[ImGuiCol_TitleBgActive]      = ImVec4(0.48f, 0.40f, 0.16f, 1.00f);
		colors[ImGuiCol_CheckMark]          = ImVec4(0.98f, 0.77f, 0.26f, 1.00f);
		colors[ImGuiCol_SliderGrab]         = ImVec4(0.88f, 0.69f, 0.24f, 1.00f);
		colors[ImGuiCol_SliderGrabActive]   = ImVec4(0.98f, 0.77f, 0.26f, 1.00f);
		colors[ImGuiCol_Button]             = ImVec4(0.98f, 0.77f, 0.26f, 0.40f);
		colors[ImGuiCol_ButtonHovered]      = ImVec4(0.98f, 0.77f, 0.26f, 1.00f);
		colors[ImGuiCol_ButtonActive]       = ImVec4(0.98f, 0.65f, 0.06f, 1.00f);
		colors[ImGuiCol_Header]             = ImVec4(0.98f, 0.77f, 0.26f, 0.31f);
		colors[ImGuiCol_HeaderHovered]      = ImVec4(0.98f, 0.77f, 0.26f, 0.80f);
		colors[ImGuiCol_HeaderActive]       = ImVec4(0.98f, 0.77f, 0.26f, 1.00f);
		colors[ImGuiCol_SeparatorHovered]   = ImVec4(0.75f, 0.54f, 0.10f, 0.78f);
		colors[ImGuiCol_SeparatorActive]    = ImVec4(0.75f, 0.54f, 0.10f, 1.00f);
		colors[ImGuiCol_ResizeGrip]         = ImVec4(0.98f, 0.77f, 0.26f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered]  = ImVec4(0.98f, 0.77f, 0.26f, 0.67f);
		colors[ImGuiCol_ResizeGripActive]   = ImVec4(0.98f, 0.77f, 0.26f, 0.95f);
		colors[ImGuiCol_Tab]                = ImVec4(0.58f, 0.47f, 0.18f, 0.86f);
		colors[ImGuiCol_TabHovered]         = ImVec4(0.98f, 0.77f, 0.26f, 0.80f);
		colors[ImGuiCol_TabActive]          = ImVec4(0.68f, 0.54f, 0.20f, 1.00f);
		colors[ImGuiCol_TabUnfocused]       = ImVec4(0.15f, 0.13f, 0.07f, 0.97f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.42f, 0.34f, 0.14f, 1.00f);
		colors[ImGuiCol_DockingPreview]     = ImVec4(0.98f, 0.77f, 0.26f, 0.70f);
		colors[ImGuiCol_TextSelectedBg]     = ImVec4(0.98f, 0.77f, 0.26f, 0.35f);
		colors[ImGuiCol_NavHighlight]       = ImVec4(0.98f, 0.77f, 0.26f, 1.00f);

		style.WindowRounding    = 2;
		style.TabRounding       = 1;
		style.ScrollbarRounding = 2;
	}
}    // namespace Style
