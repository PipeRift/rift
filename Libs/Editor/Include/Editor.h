// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Files/FileWatcher.h"

#include <AST/Tree.h>
#include <Pipe/Files/Paths.h>
#include <PipeTime.h>


namespace rift::Editor
{
	class Editor
	{
		p::FrameTime frameTime;

		bool configFileChanged = false;
		p::String configFile;

		ast::Tree ast;

		p::FileWatcher fileWatcher;

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

		int Run(p::StringView projectPath = {});

		void Tick();

		void SetUIConfigFile(p::StringView path);

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
		void CloseProject();

		// Close the editor
		void Close();

	protected:
		void UpdateConfig();
	};
}    // namespace rift::Editor
