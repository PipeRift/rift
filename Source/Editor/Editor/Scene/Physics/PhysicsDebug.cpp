// Copyright 2015-2019 Piperift - All rights reserved

#include "PhysicsDebug.h"


#if WITH_EDITOR

void PhysicsDebugHandle::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
}

void PhysicsDebugHandle::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
}

void PhysicsDebugHandle::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
{
}

void PhysicsDebugHandle::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
{
}

void PhysicsDebugHandle::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
}

void PhysicsDebugHandle::DrawTransform(const b2Transform& xf)
{
}

void PhysicsDebugHandle::DrawPoint(const b2Vec2& p, float32 size, const b2Color& color)
{
}

#endif
