// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <AST/Tree.h>
#include <Files/Paths.h>
#include <Math/Time.h>
#include <RiftContext.h>


namespace rift
{
	using namespace pipe;

	class Editor
	{
		FrameTime frameTime;

		bool configFileChanged = false;
		String configFile;

		AST::Tree ast;

	public:
#if BUILD_DEBUG
		bool showDemo    = false;
		bool showMetrics = false;
		bool showStyle   = false;
#endif


	public:
		Editor() = default;

		~Editor();

		int Run(TPtr<RiftContext> context, StringView projectPath = {});

		void Tick();

		void SetUIConfigFile(Path path);

		static Editor& Get()
		{
			static Editor instance{};
			return instance;
		}

		static AST::Tree& GetContext()
		{
			return Get().ast;
		}

		bool CreateProject(const Path& path, bool closeFirst = true);
		bool OpenProject(const Path& path, bool closeFirst = true);

		void Close();

	protected:
		void UpdateConfig();
	};
}    // namespace rift
