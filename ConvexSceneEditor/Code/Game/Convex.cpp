#include "Game/Convex.hpp"

Convex2::Convex2()
	:m_convexPoly(ConvexPoly2( { Vec2(), Vec2() } ))
{

}

Convex2::Convex2( ConvexPoly2 const& convexPoly2 )
	:m_convexPoly(convexPoly2)
	,m_convexHull(convexPoly2)
{

}

Convex2::Convex2( ConvexHull2 const& convexHull2 )
	:m_convexHull(convexHull2)
	,m_convexPoly(convexHull2)
{
}

bool Convex2::IsPointInside( Vec2 const& point ) const
{
	return IsPointInsideConvexHull2D( point, m_convexHull );
}

void Convex2::Translate( Vec2 const& offset )
{
	m_convexHull.Translate( offset );
	m_convexPoly.Translate( offset );
	m_boundingAABB.Translate( offset );
	m_boundingDiscCenter += offset;
}

void Convex2::Rotate( float degrees, Vec2 const& refPoint )
{
	m_boundingDiscCenter -= refPoint;
	m_boundingDiscCenter.RotateDegrees( degrees );
	m_boundingDiscCenter += refPoint;
	m_convexHull.Rotate( degrees, refPoint );
	m_convexPoly.Rotate( degrees, refPoint );
	RebuildBoundingBox();
}

void Convex2::Scale( float scaleFactor, Vec2 const& refPoint /*= Vec2( 0.f, 0.f ) */ )
{
	float actualFactor = (m_scale + scaleFactor) / m_scale;
	m_scale += scaleFactor;
	m_boundingRadius *= actualFactor;
	m_boundingDiscCenter -= refPoint;
	m_boundingDiscCenter *= actualFactor;
	m_boundingDiscCenter += refPoint;
	m_convexHull.Scale( actualFactor, refPoint );
	m_convexPoly.Scale( actualFactor, refPoint );
	RebuildBoundingBox();
}

bool Convex2::RayCastVsConvex2D( RayCastResult2D& out_rayCastRes, Vec2 const& startPos, Vec2 const& forwardNormal, float maxDist, bool discRejection, bool boxRejection )
{
	if (discRejection) {
		RayCastResult2D discRejectionRes;
		if (RayCastVsDisc2D( discRejectionRes, startPos, forwardNormal, maxDist, m_boundingDiscCenter, m_boundingRadius )) {
			return RayCastVsConvexHull2D( out_rayCastRes, startPos, forwardNormal, maxDist, m_convexHull );
		}
		out_rayCastRes.m_didImpact = false;
		return false;
	}
	else if (boxRejection) {
		if (RayCastVsAABB2DResultOnly( startPos, forwardNormal, maxDist, m_boundingAABB )) {
			return RayCastVsConvexHull2D( out_rayCastRes, startPos, forwardNormal, maxDist, m_convexHull );
		}
		out_rayCastRes.m_didImpact = false;
		return false;
	}
	return RayCastVsConvexHull2D( out_rayCastRes, startPos, forwardNormal, maxDist, m_convexHull );
}

void Convex2::RebuildBoundingBox()
{
	float minX = FLT_MAX, minY = FLT_MAX, maxX = -1.f, maxY = -1.f;
	for (auto const& vert : m_convexPoly.GetVertexArray()) {
		if (vert.x < minX) {
			minX = vert.x;
		}
		if (vert.x > maxX) {
			maxX = vert.x;
		}
		if (vert.y < minY) {
			minY = vert.y;
		}
		if (vert.y > maxY) {
			maxY = vert.y;
		}
	}
	m_boundingAABB = AABB2( Vec2( minX, minY ), Vec2( maxX, maxY ) );
}

