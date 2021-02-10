// Copyright 2015-2021 Piperift - All rights reserved

#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include "Window.h"

#include <imgui.h>
#include <sokol_glue.h>
#include <util/sokol_imgui.h>


void Window::Init()
{
	sg_setup(&(sg_desc){.context = sapp_sgcontext()});

	simgui_desc_t imgui_desc{};
	imgui_desc.ini_filename = "config.ini";
	simgui_setup(imgui_desc);
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	pass_action =
		(sg_pass_action){.colors[0] = {.action = SG_ACTION_CLEAR, .val = {0.1f, 0.1f, 0.1f, 1.0f}}};
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
	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
	bool open = true;
	ImGui::ShowDemoWindow(&open);
}

void Window::Close()
{
	sapp_quit();
}
