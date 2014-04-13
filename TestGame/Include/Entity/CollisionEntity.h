#pragma once
#include <TestGame/Include/Entity/RenderEntity.h>
class CollisionEntity :	public RenderEntity
{
public:
	CollisionEntity();
	~CollisionEntity();
	Position GetPosition();
	float GetWidth();
	float GetHeight();
};

