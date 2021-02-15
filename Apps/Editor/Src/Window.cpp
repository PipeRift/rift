// Copyright 2015-2021 Piperift - All rights reserved

#include "Window.h"

#include "Style.h"

#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include <imgui_internal.h>
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_glue.h>
#include <util/sokol_imgui.h>


void Window::Init()
{
	sg_desc desc{};
	desc.context = sapp_sgcontext();
	sg_setup(desc);

	simgui_desc_t imgui_desc{};
	simgui_setup(imgui_desc);
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	Style::ApplyStyle();

	pass_action           = sg_pass_action();
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
	if (configFileChanged)
	{
		ImGui::GetIO().IniFilename = configFile.c_str();
		if (FileSystem::ExistsAsFile(configFile))
		{
			// FIX: Delay this until new frame (essentially, not while already drawing)
			ImGui::LoadIniSettingsFromDisk(configFile.c_str());
		}
		else
		{
			ImGui::SaveIniSettingsToDisk(configFile.c_str());
		}
		configFileChanged = false;
	}

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
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}    // Disabled item
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

void Window::Close()
{
	sapp_quit();
}

void Window::SetUIConfigFile(Path path)
{
	configFileChanged          = true;
	configFile                 = FileSystem::ToString(path);
	ImGui::GetIO().IniFilename = configFile.c_str();
}
