// Copyright 2015-2021 Piperift - All rights reserved

#include "UI/Window.h"

#include "UI/Style.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imnodes.h>

#include <cstdio>
// OpenGL loader
#include <GL/gl3w.h>
// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

#include <Log.h>
#include <Profiler.h>
#include <Math/Color.h>


namespace Rift::UI
{
	static GLFWwindow* window = nullptr;

	void OnGl3WError(int error, const char* description)
	{
		Log::Error("Glfw Error {}: {}", error, description);
	}


	bool Init()
	{
		// Setup window
		glfwSetErrorCallback(OnGl3WError);
		if (!glfwInit())
		{
			return false;
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
			return false;
		}

		glfwMakeContextCurrent(window);
		glfwSwapInterval(1);    // Enable vsync

		// Initialize OpenGL loader
		if (gl3wInit() != 0)
		{
			Log::Error("Failed to initialize OpenGL loader (gl3w)!\n");
			return false;
		}

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImNodes::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;    // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		Style::PushGeneralStyle();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding              = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init(glsl_version);
		return true;
	}

	void Shutdown()
	{
		Style::PopGeneralStyle();
		if (window)
		{
			// Cleanup
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImNodes::DestroyContext();
			ImGui::DestroyContext();

			glfwDestroyWindow(window);
			glfwTerminate();
			window = nullptr;
		}
	}

	void PreFrame()
	{
		ZoneScopedN("PreFrame");
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void Render()
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
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
		glfwSwapBuffers(window);
	}

	void Close()
	{
		glfwSetWindowShouldClose(window, true);
	}

	bool WantsToClose()
	{
		return glfwWindowShouldClose(window);
	}

	GLFWwindow* GetWindow()
	{
		return window;
	}
}    // namespace Rift::UI
