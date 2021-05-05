// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Strings/String.h"

#include <CoreObject.h>


namespace Rift
{
	class BaseEditor : public Rift::Object
	{
		CLASS(BaseEditor, Rift::Object)

	public:
		BaseEditor() : Super() {}
		virtual ~BaseEditor() {}

		virtual StringView GetWindowId()
		{
			return {};
		}
	};
}    // namespace Rift
