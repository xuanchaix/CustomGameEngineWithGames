#pragma once
#include "Game/GameCommon.hpp"

struct Convex2 {
public:
	Convex2();
	Convex2( ConvexHull2 const& convexHull2 );
	Convex2( ConvexPoly2 const& convexPoly2 );

	bool IsPointInside( Vec2 const& point ) const;
	void Translate( Vec2 const& offset );
	void Rotate( float degrees, Vec2 const& refPoint = Vec2( 0.f, 0.f ) );
	void Scale( float scaleFactor, Vec2 const& refPoint = Vec2( 0.f, 0.f ) );
	bool RayCastVsConvex2D( RayCastResult2D& out_rayCastRes, Vec2 const& startPos, Vec2 const& forwardNormal, float maxDist, bool discRejection=true, bool boxRejection=false );
	void RebuildBoundingBox();

	ConvexHull2 m_convexHull;
	ConvexPoly2 m_convexPoly;
	AABB2 m_boundingAABB;
	Vec2 m_boundingDiscCenter;
	float m_boundingRadius;
	float m_scale = 1.f;
	bool m_symmetricQuadTreeFlag = false;
};