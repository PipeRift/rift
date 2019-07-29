// © 2019 Miguel Fernández Arce - All rights reserved

#pragma once

#include "CoreObject.h"
#include "Core/Assets/AssetData.h"
#include "Rendering/Data/CubeTextureData.h"


class CubeTexture : public AssetData
{
	CLASS(CubeTexture, AssetData)

private:

	PROP(String, positiveX)
	String positiveX;

	PROP(String, negativeX)
	String negativeX;

	PROP(String, positiveY)
	String positiveY;

	PROP(String, negativeY)
	String negativeY;

	PROP(String, positiveZ)
	String positiveZ;

	PROP(String, negativeZ)
	String negativeZ;

	CubeTextureData data;


public:

	virtual bool PostLoad() override;
	virtual void OnUnload() override;

	const CubeTextureData& GetTextureData() const;
};
