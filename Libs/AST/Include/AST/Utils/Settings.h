// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include <Pipe/Core/String.h>
#include <Pipe/Files/Files.h>
#include <Pipe/Serialize/Formats/JsonFormat.h>
#include <Pipe/Serialize/Serialization.h>
#include <PipeECS.h>


namespace rift
{
	p::String GetUserSettingsPath();

	template<typename T>
	T& GetUserSettings()
	{
		static p::TOwnPtr<T> instance;
		if (!instance)
		{
			instance = p::MakeOwned<T>();

			p::String path = GetUserSettingsPath();
			if (!p::Exists(path))
			{
				p::CreateFolder(path);
			}

			p::AppendToPath(path, p::GetTypeName<T>(false));
			path.append(".json");
			if (!p::Exists(path))
			{
				p::SaveStringFile(path, "{}");
			}

			p::String data;
			if (p::LoadStringFile(path, data))
			{
				p::JsonFormatReader reader{data};
				reader.GetReader().Serialize(*instance);
			}
		}
		return *instance.Get();
	}
}    // namespace rift
