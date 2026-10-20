// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "UI/Texture.h"

#include <GL/gl3w.h>
#include <Pipe/Core/Log.h>
#include <Pipe/Files/Files.h>
#include <PipeStrings.h>
#include <stb_image.h>


namespace rift::UI
{
	ImTextureID LoadTexture(p::StringView path)
	{
		int width = 0, height = 0, channels = 0;
		stbi_uc* pixels = stbi_load(p::String{path}.c_str(), &width, &height, &channels, 4);
		if (!pixels)
		{
			p::Error("Texture '{}' could not be loaded", path);
			return ImTextureID_Invalid;
		}

		GLuint textureId = 0;
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(
		    GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		glBindTexture(GL_TEXTURE_2D, 0);

		stbi_image_free(pixels);
		return static_cast<ImTextureID>(textureId);
	}

	void UnloadTexture(ImTextureID textureId)
	{
		if (textureId == ImTextureID_Invalid)
		{
			return;
		}
		glDeleteTextures(1, reinterpret_cast<GLuint*>(&textureId));
	}
}    // namespace rift::UI
