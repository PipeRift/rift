// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "EditorData.h"

#include <Misc/Time.h>


namespace Rift
{
	class Editor
	{
		FrameTime frameTime;

		EditorData editorData;

		bool configFileChanged = false;
		String configFile;

		AST::Tree ast;


	public:
		Editor() = default;

		~Editor();

		int Run();

		void Close();

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


	protected:
		void UpdateConfig();
	};
}    // namespace Rift
