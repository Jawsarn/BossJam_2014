#pragma once

#include <Jamgine\Include\Shared.h>

namespace Jamgine
{
	class Rectangle
	{
	public:
		Rectangle();

		/* Comment */
		Rectangle(Position l_position, float l_width, float l_height);

		~Rectangle();
			
		Position m_position;
		float m_width, m_height;			
	};
}