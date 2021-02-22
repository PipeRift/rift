// Copyright 2015-2021 Piperift - All rights reserved

#include "Window.h"

#include "Style.h"

#include <Math/Color.h>
#include <Profiler.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <cstdio>

// OpenGL loader
#include <GL/gl3w.h>

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>


int Window::Run()
{
	// Setup window
	glfwSetErrorCallback(Window::OnGl3WError);
	if (!glfwInit())
	{
		return 1;
	}

	// Decide GL+GLSL versions
#ifdef __APPLE__
	// GL 3.2 + GLSL 150
	const char* glsl_version = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);    // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);              // Required on Mac
#else
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	// glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

	window = glfwCreateWindow(1280, 900, "Rift", nullptr, nullptr);
	if (window == nullptr)
	{
		return 1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);    // Enable vsync

	// Initialize OpenGL loader
	if (gl3wInit() != 0)
	{
		Log::Error("Failed to initialize OpenGL loader (gl3w)!\n");
		return 1;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;    // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	Style::ApplyStyle();

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding              = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple
	// fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the
	// font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in
	// your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture
	// when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below
	// will call.
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to
	// write a double backslash \\ !
	// io.Fonts->AddFontDefault();
	// io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	// io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	// io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	// io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	// ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL,
	// io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != NULL);

	while (!glfwWindowShouldClose(window))
	{
		frameTime.Tick();
		{
			ZoneScopedN("PreFrame");
			glfwPollEvents();

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
		}

		Tick(frameTime.GetDeltaTime());

		{
			ZoneScopedNC("Render", 0xA245D1);

			ImGui::Render();
			i32 display_w, display_h;
			glfwGetFramebufferSize(window, &display_w, &display_h);
			glViewport(0, 0, display_w, display_h);

			static constexpr Rift::LinearColor clearColor{0.1f, 0.1f, 0.1f, 1.00f};
			glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
			glClear(GL_COLOR_BUFFER_BIT);
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			// Update and Render additional Platform Windows
			// (Platform functions may change the current OpenGL context, so we save/restore it to
			// make it easier to paste this code elsewhere.
			//  For this specific demo app we could also call glfwMakeContextCurrent(window)
			//  directly)
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				GLFWwindow* backup_current_context = glfwGetCurrentContext();
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
				glfwMakeContextCurrent(backup_current_context);
			}
			glfwSwapBuffers(window);
		}
		FrameMark;

		frameTime.PostTick();
	}
	return 0;
}

Window::~Window()
{
	if (window)
	{
		// Cleanup
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		glfwDestroyWindow(window);
		glfwTerminate();
		window = nullptr;
	}
}

void Window::Close()
{
	glfwSetWindowShouldClose(window, true);
}

void Window::SetUIConfigFile(Path path)
{
	if (window)
	{
		configFileChanged          = true;
		configFile                 = FileSystem::ToString(path);
		ImGui::GetIO().IniFilename = configFile.c_str();
	}
}

void Window::Tick(float /*deltaTime*/)
{
	ZoneScopedN("Tick");
	UpdateConfig();

	rootEditor.Draw();

	static bool open = true;
	ImGui::ShowDemoWindow(&open);
}

void Window::UpdateConfig()
{
	if (configFileChanged)
	{
		ImGui::GetIO().IniFilename = configFile.c_str();
		if (configFile.empty())
		{
			return;
		}

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
}

void Window::OnGl3WError(int error, const char* description)
{
	Log::Error("Glfw Error {}: {}", error, description);
}