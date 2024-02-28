// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "AST/Tree.h"
#include "AST/TypeRef.h"

#include <Pipe/Core/String.h>
#include <Pipe/Serialize/Serialization.h>
#include <PipeECS.h>


namespace rift::ast
{
	p::String GetUserSettingsPath();

	template<typename T>
	T* GetUserSettings()
	{
		static TOwnPtr<T> instance;
		if (!instance)
		{
			p::String path = GetUserSettingsPath();
			String data;
			// if (p::LoadStringFile(path, data))
			//{
			//	instance = MakeOwned<T>();
			//	p::JsonFormatReader reader{data};
			//	reader.GetReader().Serialize(*instance);
			// }
		}
		return instance.Get();
	}
}    // namespace rift::ast
