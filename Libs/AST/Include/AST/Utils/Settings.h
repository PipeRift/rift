// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include "AST/Utils/Settings.h"
#include "Pipe/Files/Paths.h"

#include <Pipe/Core/String.h>
#include <Pipe/Files/Files.h>
#include <PipeECS.h>
#include <PipeSerialize.h>


namespace rift
{
	p::String GetUserSettingsPath(p::StringView name);

	template<typename T>
	T& GetUserSettings()
	{
		static p::TOwnPtr<T> instance;
		if (!instance)
		{
			instance = p::MakeOwned<T>();

			p::String filePath = GetUserSettingsPath(p::GetTypeName<T>(false));
			if (!p::Exists(filePath))
			{
				SaveUserSettings<T>();
			}
			else
			{
				p::String data;
				if (p::LoadStringFile(filePath, data))
				{
					p::JsonFormatReader reader{data};
					reader.GetReader().Serialize(*instance);
				}
			}
		}
		return *instance.Get();
	}

	template<typename T>
	void SaveUserSettings()
	{
		auto& instance = GetUserSettings<T>();
		p::JsonFormatWriter writer{};
		writer.GetWriter().Serialize(instance);

		p::String filePath       = GetUserSettingsPath(p::GetTypeName<T>(false));
		p::StringView folderPath = p::GetParentPath(filePath);
		if (!p::Exists(folderPath))
		{
			p::CreateFolder(folderPath);
		}
		p::SaveStringFile(filePath, writer.ToString());
	}
}    // namespace rift
