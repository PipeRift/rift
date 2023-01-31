// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <AST/Tree.h>
#include <Pipe/Files/Paths.h>
#include <Pipe/Math/FrameTime.h>


namespace rift::Editor
{
	using namespace p;

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

		int Run(StringView projectPath = {});

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

		bool CreateProject(p::StringView path, bool closeFirst = true);
		bool OpenProject(p::StringView path, bool closeFirst = true);

		void Close();

	protected:
		void UpdateConfig();
	};
}    // namespace rift::Editor
