// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "BaseStruct.h"
#include "CoreEngine.h"
#include "Memory/Allocator.h"

#include <EASTL/type_traits.h>


namespace VCLang
{
	namespace Refl
	{
		class Class;
	}
	class Object;

	template <typename Type>
	struct Ptr;


	class CORE_API BaseObject : public BaseStruct
	{
	protected:
		BaseObject() : BaseStruct() {}

	public:
		void StartDestroy()
		{
			BeforeDestroy();
		}

		Refl::Class* GetType() const;
		Ptr<Object> Self() const;

	protected:
		virtual void BeforeDestroy() {}
	};


	template <typename Type>
	using IsObject = std::is_base_of<BaseObject, Type>;
}	 // namespace VCLang
