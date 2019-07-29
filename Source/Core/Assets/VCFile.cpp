// © 2019 Miguel Fernández Arce - All rights reserved

#include "CubeTexture.h"
#include <SDL_surface.h>
#include <SDL_image.h>

#include "Core/Engine.h"
#include "Core/Assets/AssetPtr.h"
#include "Core/Files/FileSystem.h"
#include "Rendering/Commands/TextureCommands.h"


bool CubeTexture::PostLoad()
{
	TArray<String*> texturePaths
	{
		&positiveX, &negativeX,
		&positiveY, &negativeY,
		&positiveZ, &negativeZ
	};

	data = {};

	bool bSuccess = true;

	for(u8 i = 0; i < texturePaths.Size(); ++i)
	{
		String* texturePath = texturePaths[i];

		const Path rawTexturePath = FileSystem::FromString(*texturePath);
		if(rawTexturePath.empty() || !FileSystem::IsAssetPath(rawTexturePath))
		{
			Log::Error("CubeTexture used missing texture '%s'", texturePath->c_str());
			bSuccess = false;
			continue;
		}

		// Load texture from absolute path
		SDL_Surface* rawImg = IMG_Load(FileSystem::ToString(FileSystem::GetAssetsPath() / rawTexturePath).c_str());
		// Copy texture data
		bSuccess &= rawImg && data.textures[i].FromSurface(rawImg);
		SDL_FreeSurface(rawImg);
	}

	if (bSuccess)
	{
		QueueRenderCommand<LoadCubeTextureCommand>(TAssetPtr<CubeTexture>{ Self<CubeTexture>() });
	}

	return bSuccess;
}

void CubeTexture::OnUnload()
{
	QueueRenderCommand<FreeCubeTextureCommand>(GetInfo());
}

const CubeTextureData& CubeTexture::GetTextureData() const
{
	return data;
}
