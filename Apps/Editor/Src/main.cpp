// Copyright 2015-2021 Piperift - All rights reserved

#include <Platform/Platform.h>
#include <imgui.h>

#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_glue.h>
#include <sokol_time.h>
#include <util/sokol_imgui.h>


using namespace Rift;

sg_pass_action pass_action;
u64 lastTime;

void Init(void)
{
	stm_setup();
	lastTime = stm_now();

	sg_setup(&(sg_desc){.context = sapp_sgcontext()});
	pass_action =
		(sg_pass_action){.colors[0] = {.action = SG_ACTION_CLEAR, .val = {1.0f, 0.0f, 0.0f, 1.0f}}};

	simgui_desc_t imgui_desc{};
	simgui_setup(imgui_desc);
}

void Frame(void)
{
	u64 newTime = stm_now();
	const float deltaTime = stm_sec(lastTime - newTime);
	lastTime = newTime;

	simgui_new_frame(sapp_width(), sapp_height(), deltaTime);

	bool open = true;
	ImGui::ShowDemoWindow(&open);

	float g = pass_action.colors[0].val[1] + 0.01f;
	pass_action.colors[0].val[1] = (g > 1.0f) ? 0.0f : g;
	sg_begin_default_pass(&pass_action, sapp_width(), sapp_height());
	simgui_render();
	sg_end_pass();
	sg_commit();
}

void Cleanup(void)
{
	sg_shutdown();
}

void Event(const sapp_event* ev)
{
	simgui_handle_event(ev);
}

sapp_desc sokol_main(int argc, char* argv[])
{
	return (sapp_desc){
		.init_cb = Init,
		.frame_cb = Frame,
		.cleanup_cb = Cleanup,
		.event_cb = Event,
		.width = 1200,
		.height = 900,
		.window_title = "Clear (sokol app)",
	};
}
