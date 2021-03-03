// Copyright 2015-2021 Piperift - All rights reserved

#include "Window.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imnodes.h>

#include <cstdio>

// OpenGL loader
#include <GL/gl3w.h>

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>
#include <Log.h>


namespace Rift::UI
{
	static GLFWwindow* window = nullptr;


	bool Init()
	{
		// Setup window
		// glfwSetErrorCallback(Window::OnGl3WError);
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
		imnodes::Initialize();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;    // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		// Style::ApplyStyle();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding              = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
	}
}    // namespace Rift::UI
