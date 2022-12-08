// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Plugin.h"
#include "View.h"

#include <Pipe/Reflect/Class.h>
#include <Pipe/Reflect/ClassType.h>


namespace rift
{
	class Rift : public Class
	{
		CLASS(Rift, Class)

	protected:
		TMap<ClassType*, TOwnPtr<Plugin>> plugins;
		TArray<View> views;


	public:
		template<typename T>
		void EnablePlugin()
		{
			plugins.Insert(T::GetStaticType(), MakeOwned<T>());
		}

		template<typename T>
		void DisablePlugin()
		{
			plugins.Remove(T::StaticType());
		}

		template<typename T>
		void AddView(T view)
		{
			views.Add(Move(view));
		}
	};
}    // namespace rift
