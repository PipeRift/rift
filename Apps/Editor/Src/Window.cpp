// Copyright 2015-2021 Piperift - All rights reserved

#define SOKOL_IMPL
#define SOKOL_GLCORE33

#include "Window.h"

#include <imgui_internal.h>
#include <sokol_glue.h>
#include <util/sokol_imgui.h>


void Window::Init()
{
	sg_desc desc{};
	desc.context = sapp_sgcontext();
	sg_setup(desc);

	simgui_desc_t imgui_desc{};
	imgui_desc.ini_filename = "config.ini";
	simgui_setup(imgui_desc);
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ApplyStyle();

	pass_action = sg_pass_action();
	pass_action.colors[0] = {SG_ACTION_CLEAR, {0.1f, 0.1f, 0.1f, 1.0f}};
}

void Window::Frame()
{
	frameTime.Tick();

	simgui_new_frame(sapp_width(), sapp_height(), frameTime.GetDeltaTime());

	Tick(frameTime.GetDeltaTime());

	sg_begin_default_pass(&pass_action, sapp_width(), sapp_height());
	simgui_render();
	sg_end_pass();
	sg_commit();

	frameTime.PostTick();
}

void Window::Event(const sapp_event* ev)
{
	simgui_handle_event(ev);
}

void Window::Shutdown()
{
	sg_shutdown();
}

void Window::Tick(float /*deltaTime*/)
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open Project")) {}
			if (ImGui::MenuItem("Open File")) {}
			if (ImGui::MenuItem("Save File", "CTRL+S")) {}
			if (ImGui::MenuItem("Save All", "CTRL+SHFT+S")) {}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}	   // Disabled item
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X")) {}
			if (ImGui::MenuItem("Copy", "CTRL+C")) {}
			if (ImGui::MenuItem("Paste", "CTRL+V")) {}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	project.Draw();

	static bool open = true;
	ImGui::ShowDemoWindow(&open);
}

void Window::ApplyStyle()
{
	ImGui::StyleColorsDark();
	auto& style = ImGui::GetStyle();

	ImVec4* colors = style.Colors;
	colors[ImGuiCol_FrameBg] = ImVec4(0.48f, 0.40f, 0.16f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.98f, 0.77f, 0.26f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.98f, 0.77f, 0.26f, 0.67f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.48f, 0.40f, 0.16f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.98f, 0.77f, 0.26f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.88f, 0.69f, 0.24f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.98f, 0.77f, 0.26f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.98f, 0.77f, 0.26f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.98f, 0.77f, 0.26f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.98f, 0.65f, 0.06f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.98f, 0.77f, 0.26f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.98f, 0.77f, 0.26f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.98f, 0.77f, 0.26f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.75f, 0.54f, 0.10f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.75f, 0.54f, 0.10f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.98f, 0.77f, 0.26f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.98f, 0.77f, 0.26f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.98f, 0.77f, 0.26f, 0.95f);
	colors[ImGuiCol_Tab] = ImVec4(0.58f, 0.47f, 0.18f, 0.86f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.98f, 0.77f, 0.26f, 0.80f);
	colors[ImGuiCol_TabActive] = ImVec4(0.68f, 0.54f, 0.20f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.13f, 0.07f, 0.97f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.42f, 0.34f, 0.14f, 1.00f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.98f, 0.77f, 0.26f, 0.70f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.98f, 0.77f, 0.26f, 0.35f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.98f, 0.77f, 0.26f, 1.00f);

	style.WindowRounding = 2;
	style.ScrollbarRounding = 2;
}

void Window::Close()
{
	sapp_quit();
}
