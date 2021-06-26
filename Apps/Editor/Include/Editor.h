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

	protected:
		void UpdateConfig();
	};
}    // namespace Rift
