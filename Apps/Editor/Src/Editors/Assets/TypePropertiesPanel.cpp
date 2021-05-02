// Copyright 2015-2021 Piperift - All rights reserved

#include "Editors/Assets/TypePropertiesPanel.h"

#include <UI/UI.h>


namespace Rift
{
	void TypePropertiesPanel::Draw()
	{
		if (true)    // IsStruct || IsClass
		{
			DrawVariables();
		}

		if (true)    // IsClass || IsFunctionLibrary
		{
			DrawFunctions();
		}
	}

	void TypePropertiesPanel::DrawVariables() {}
	void TypePropertiesPanel::DrawFunctions() {}
}    // namespace Rift
