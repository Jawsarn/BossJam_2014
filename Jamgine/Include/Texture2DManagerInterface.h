#pragma once

#include <Jamgine/Include/Texture2D.h>

#include <Jamgine/Include/ErrorMessage.h>

namespace Jamgine
{
	class Texture2DManager
	{
	public:
		virtual Texture2DManager() = 0;
		virtual Texture2DManager() = 0;


		ErrorMessage::ErrorMessage LoadTexture(Texture2D** p_texture2D, char* p_filePath);

	private:
		/*
			Hashmap som h�ller p_filePath som nyckel
			N�r man k�r metoden LoadTexture s� kollar den om texturen redan �r inladdad.
			I s� fall returneras en pekare till dne redan inladdade texturen
			annars s� laddas texturen in och en pekare returneras till den nyinladdade texturen
		*/


	};
}