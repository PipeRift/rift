// Copyright 2015-2020 Piperift - All rights reserved
#pragma once


namespace Rift
{
	namespace Refl
	{
		class Struct;
	}

	struct BaseStruct
	{
		BaseStruct() = default;
		virtual ~BaseStruct() {}

		Refl::Struct* GetType() const;

		/* TODO: Support different arenas for object/struct types
		void* operator new(size_t size)
		{
		}
		void operator delete(void* p, std::size_t size)
		{
		}*/
	};
}	 // namespace Rift