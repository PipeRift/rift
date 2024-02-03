// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Files/FileWatcher.h"

#include <AST/Tree.h>
#include <Pipe/Files/Paths.h>
#include <PipeTime.h>


namespace rift::Editor
{
	using namespace p;

	class Editor
	{
		FrameTime frameTime;

		bool configFileChanged = false;
		String configFile;

		ast::Tree ast;

		FileWatcher fileWatcher;

	public:
		bool bFilesDirty = true;
#if P_DEBUG
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

		static ast::Tree& GetContext()
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
