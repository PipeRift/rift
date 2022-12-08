// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "View.h"

#include <Pipe/Reflect/Class.h>
#include <Pipe/Reflect/ClassType.h>


namespace rift
{
	class Rift : public p::Class
	{
		CLASS(Rift, p::Class)

	protected:
		p::TMap<p::ClassType*, p::TOwnPtr<class Module>> modules;
		p::TArray<View> views;


	public:
		template<typename T>
		void EnableModule()
		{
			EnableModule(T::GetStaticType());
		}

		template<typename T>
		void DisableModule()
		{
			DisableModule(T::GetStaticType());
		}

		template<typename T>
		void AddView(T view)
		{
			views.Add(Move(view));
		}

		void EnableModule(p::ClassType* type);
		void DisableModule(p::ClassType* type);
	};
}    // namespace rift
