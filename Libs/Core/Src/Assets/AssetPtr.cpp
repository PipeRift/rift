// Copyright 2015-2020 Piperift - All rights reserved
#include "Assets/AssetPtr.h"

#if WITH_EDITOR
#	include "UI/Widgets/Properties/AssetInfoPropertyWidget.h"

namespace Rift
{
	Class* BaseAssetPtr::GetDetailsWidgetClass()
	{
		return AssetInfoPropertyWidget::Type();
	}
}	 // namespace Rift
#endif
