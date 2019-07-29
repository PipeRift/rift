// Copyright 2015-2019 Piperift - All rights reserved

#include "Renderer.h"

#include <SDL_opengl.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_sdl.h>
#include <imgui/ImGuizmo.h>
#include <tracy/TracyOpenGL.hpp>

#include "World.h"
#include "Core/Log.h"
#include "Tools/Profiler.h"
#include "Core/Engine.h"
#include "Interface/OpenGL.h"


#if PLATFORM_APPLE
const char* Renderer::glslVersion{ "#version 150" };
#else
const char* Renderer::glslVersion{ "#version 130" };
#endif

Renderer::Renderer()
	: Super()
	, window{nullptr}
{

}

bool Renderer::Initialize()
{
	engine = *GEngine;

#if PLATFORM_APPLE
	// GL 3.2 Core + GLSL 150
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
#endif
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	// Setup window
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	SDL_DisplayMode current;
	SDL_GetCurrentDisplayMode(0, &current);

	window = SDL_CreateWindow(
		"Rift Engine",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		SCREEN_WIDTH, SCREEN_HEIGHT,
#if WITH_EDITOR
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED
#else
		SDL_WINDOW_OPENGL | SDL_WINDOW_MAXIMIZED
#endif
	);
	if (!window)
	{
		Log::Message("Could not create a window");
		return false;
	}

	gl_context = SDL_GL_CreateContext(window);
	SDL_GL_SetSwapInterval(0); // 1 Enables vsync

	if (gl3wInit() != 0)
	{
		Log::Message("Could not initialize OpenGL");
		return false;
	}

	PrepareUI();
	TracyGpuContext(gl_context);

	return true;
}

void Renderer::PrepareUI()
{
	// Setup Dear ImGui binding
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL3_Init(glslVersion);
}

void Renderer::PreTick()
{
	ScopedGraphicsZone("Prepare Frame");
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);

#if WITH_EDITOR
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
#endif //WITH_EDITOR
}

void Renderer::Render()
{
	ScopedGraphicsZone("Render");

	SDL_GL_MakeCurrent(window, gl_context);

#if WITH_EDITOR
	// Use ImGui viewport size
	ImGuiViewport* vp = ImGui::GetMainViewport();
	v2_u32 viewportSize{ (u32)vp->Size.x, (u32)vp->Size.y };
#else
	SDL_DisplayMode displayMode;
	SDL_GetCurrentDisplayMode(0, &displayMode);
	v2_u32 viewportSize{ (u32)displayMode.w, (u32)displayMode.h };
#endif

	glViewport(0, 0, viewportSize.x, viewportSize.y);
	glClearColor(0.7f, 0.4f, 0.4f, 1);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);

	// World Render
	{
		ScopedGraphicsZone("World");

		// Reset render data
		render.NewFrame(viewportSize);

		engine->GetRenderFrame().ExecuteCommands(render);
	}

	glCheckError();
}

void Renderer::RenderUI()
{
#if WITH_EDITOR
	// UI Render
	ScopedGraphicsZone("Render UI");
	ImDrawData* drawData = ImGui::GetDrawData();
	if (!drawData)
	{
		Log::Error("UI Draw Data should never be invalid");
		return;
	}

	ImGui_ImplOpenGL3_RenderDrawData(drawData);

	// Update and Render additional Platform Windows
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
#endif
}

void Renderer::SwapWindow()
{
	ScopedZone("Swap & VSync", D15545);
	SDL_GL_SwapWindow(window);
}

void Renderer::BeforeDestroy()
{
	Super::BeforeDestroy();

	if (window)
	{
		SDL_DestroyWindow(window);
	}
}
