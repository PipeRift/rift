// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <AST/Tree.h>
#include <Files/Paths.h>
#include <Misc/Time.h>


namespace Rift
{
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

		static AST::Tree& GetAST()
		{
			return Get().ast;
		}

		bool OpenProject(const Path& path, bool closeFirst = true);

		void Close();

	protected:
		void UpdateConfig();
	};
}    // namespace Rift
