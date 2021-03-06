#pragma once
#include <Jamgine/Include/Shared.h>
#include <Jamgine/Include/Texture/Texture2DInterface.h>
#include <Jamgine/Include/Camera.h>

namespace Jamgine
{
	struct Data_Send;

	class JamgineEngine
	{
		
	public:
		// This engine is a singleton, only one instance can be created. If you wish to change 3D API, you'll have to Release the current engine and Create it with new flags.
		static ErrorMessage CreateEngine(JamgineEngine** p_jamgineEngine, GraphicalSystem p_graphicalSystem);
		static ErrorMessage ReleaseEngine(); 
		
		// As long as the data is packed correctly it can be sent as a void*. This has the benefit of skipping one include file.
		virtual ErrorMessage Initialize(void* p_data) = 0;
		
		// If Jamgine/Include/'API'Shared has been included, a definition of the struct exists.
		virtual ErrorMessage Initialize(Jamgine::Data_Send p_data) = 0;

		// Because of a map, a texture will actually only be loaded once and the same pointer will be returned.
		virtual ErrorMessage LoadTexture(Texture2DInterface** p_texture2DInterface, char* p_filePath) = 0;

		//virtual void PreRender() = 0;
		virtual	void Render(Position p_position, Position p_origin,	Position p_textureOffset, Texture2DInterface* p_texture, SpriteEffect p_spriteEffect, float p_width, float p_height, float p_depth, float p_rotation,bool p_hasTransparent, Position p_textureDelta)= 0;

		virtual void Render(
			Position p_position,
			Position p_origin,
			Position p_textureOffset,
			Texture2DInterface* p_texture,
			float p_width,
			float p_height,
			float p_depth,
			float p_rotation)= 0;
		virtual void Render(Position p_position,
			Position p_textureOffset,
			Texture2DInterface* p_texture,
			float p_width,
			float p_height,
			float p_depth) = 0;
		virtual void Render(Position p_position, Position p_textureOffset,
			Texture2DInterface* p_texture,
			SpriteEffect p_spriteEffect,
			float p_width,
			float p_height,
			float p_depth
				) = 0;

		// This function is called once per draw, this call actually renders the sprites to the backbuffer. Camera is used for offset
		virtual void PostRender(Camera* p_camera) = 0;

	private:
		static JamgineEngine* m_jamgineEngine;		
	
	protected:
	};
}