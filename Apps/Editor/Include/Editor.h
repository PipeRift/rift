// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Editors/RootEditor.h"

#include <Misc/Time.h>

namespace Rift
{
	class Editor
	{
		FrameTime frameTime;

		RootEditor rootEditor;

		bool configFileChanged = false;
		String configFile;


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

		RootEditor& GetRootEditor()
		{
			return rootEditor;
		}

	protected:
		void Tick(float deltaTime);

		void UpdateConfig();

		static void OnGl3WError(int error, const char* description);
	};
}    // namespace Rift
