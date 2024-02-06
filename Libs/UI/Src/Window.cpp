// Copyright 2015-2023 Piperift - All rights reserved

#include "UI/Window.h"

#include "UI/Inspection.h"
#include "UI/Style.h"
#include "UI/UI.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <cstdio>

// OpenGL loader
#include <GL/gl3w.h>
// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>
#include <Pipe/Core/Log.h>
#include <Pipe/Files/Paths.h>
#include <PipeColor.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


namespace rift::UI
{
	using namespace p;


	static GLFWwindow* gWindow = nullptr;

	void OnGl3WError(int error, const char* description)
	{
		p::Error("Glfw Error {}: {}", error, p::StringView{description});
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
		const char* glslVersion = "#version 150";
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);    // 3.2+ only
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);              // Required on Mac
#else
		// GL 3.0 + GLSL 130
		const char* glslVersion = "#version 130";
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
		// glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
		// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

		gWindow = glfwCreateWindow(1280, 900, "Rift", nullptr, nullptr);
		if (gWindow == nullptr)
		{
			return false;
		}

		glfwMakeContextCurrent(gWindow);
		glfwSwapInterval(1);    // Enable vsync

		// Initialize OpenGL loader
		if (gl3wInit() != 0)
		{
			p::Error("Failed to initialize OpenGL loader (gl3w)!\n");
			return false;
		}

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable
		                | ImGuiConfigFlags_ViewportsEnable;
		io.ConfigDockingAlwaysTabBar = true;

		UI::PushGeneralStyle();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding              = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// Setup Pipe/Core/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(gWindow, true);
		ImGui_ImplOpenGL3_Init(glslVersion);

		SetWindowIcon();

		RegisterCoreKeyValueInspections();
		return true;
	}

	void Shutdown()
	{
		UI::PopGeneralStyle();
		if (gWindow)
		{
			// Cleanup
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();

			glfwDestroyWindow(gWindow);
			glfwTerminate();
			gWindow = nullptr;
		}
	}

	void PreFrame()
	{
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void Render()
	{
		ImGui::Render();
		i32 displayW, displayH;
		glfwGetFramebufferSize(gWindow, &displayW, &displayH);
		glViewport(0, 0, displayW, displayH);

		static constexpr LinearColor clearColor{0.1f, 0.1f, 0.1f, 1.00f};
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
			GLFWwindow* backupCurrentContext = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backupCurrentContext);
		}
		glfwSwapBuffers(gWindow);
	}

	void Close()
	{
		glfwSetWindowShouldClose(gWindow, true);
	}

	bool WantsToClose()
	{
		return glfwWindowShouldClose(gWindow);
	}

	GLFWwindow* GetWindow()
	{
		return gWindow;
	}

	void SetWindowIcon()
	{
		p::String icon64Path = p::JoinPaths(p::GetBasePath(), "Resources/Editor/Icons/Logo_64.png");
		p::String icon128Path =
		    p::JoinPaths(p::GetBasePath(), "Resources/Editor/Icons/Logo_128.png");
		p::String icon256Path =
		    p::JoinPaths(p::GetBasePath(), "Resources/Editor/Icons/Logo_256.png");
		GLFWimage images[3];
		images[0].pixels =
		    stbi_load(icon64Path.c_str(), &images[0].width, &images[0].height, nullptr, 0);
		images[1].pixels =
		    stbi_load(icon128Path.c_str(), &images[1].width, &images[1].height, nullptr, 0);
		images[2].pixels =
		    stbi_load(icon256Path.c_str(), &images[2].width, &images[2].height, nullptr, 0);
		if (!images[0].pixels || !images[1].pixels || !images[2].pixels)
		{
			p::Error("Window icon couldn't be loaded");
			return;
		}
		glfwSetWindowIcon(gWindow, 3, images);

		stbi_image_free(images[0].pixels);
		stbi_image_free(images[1].pixels);
		stbi_image_free(images[2].pixels);
	}
}    // namespace rift::UI
