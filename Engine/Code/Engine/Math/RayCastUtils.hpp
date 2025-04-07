#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"

struct AABB2;
struct AABB3;
struct OBB3;
struct Plane3;
struct ConvexHull2;

struct Ray2D {
public:
	Ray2D( Vec2 const& startPos, Vec2 const& forwardNormal, float maxDist );
	Ray2D( Vec2 const& startPos, Vec2 const& endPos );
	Ray2D( Vec2 const& startPos, float orientationDegrees, float maxDist );
public:
	Vec2 m_startPos;
	float m_maxDist;
	Vec2 m_forwardNormal;
};

struct Ray3D {
public:
	Ray3D( Vec3 const& startPos, Vec3 const& forwardNormal, float maxDist );
	Ray3D( Vec3 const& startPos, Vec3 const& endPos );
public:
	Vec3 m_startPos;
	float m_maxDist;
	Vec3 m_forwardNormal;
};

struct RayCastResult2D
{
	bool m_didImpact = false;
	float m_impactDist = 0.f;
	Vec2 m_impactPos;
	Vec2 m_impactNormal;
	Vec2 m_rayForwardNormal;
	Vec2 m_rayStartPos;
	float m_rayMaxLength = 1.f;
};

struct RayCastResult3D
{
	bool m_didImpact = false;
	float m_impactDist = 0.f;
	Vec3 m_impactPos;
	Vec3 m_impactNormal;
	Vec3 m_rayStartPos;
	Vec3 m_rayForwardNormal;
	float m_rayMaxLength = 1.f;
};

/// Ray cast to hit a 2D disc, returns whether the ray hits the disc
bool RayCastVsDisc2D( RayCastResult2D& out_rayCastRes, Vec2 const& startPos, Vec2 const& forwardNormal, float maxDist, Vec2 const& discCenter, float discRadius );
/// Ray cast to hit a 2D disc, returns whether the ray hits the disc
bool RayCastVsDisc2D( RayCastResult2D& out_rayCastRes, Ray2D const& ray2D, Vec2 const& discCenter, float discRadius );
/// Ray cast to hit a 2D disc, returns whether the ray hits the disc
bool RayCastVsDisc2D( RayCastResult2D& out_rayCastRes, Vec2 const& startPos, float forwardDegrees, float maxDist, Vec2 const& discCenter, float discRadius );
/// Ray cast to hit a 2D disc, returns whether the ray hits the disc
bool RayCastVsDisc2D( RayCastResult2D& out_rayCastRes, Vec2 const& startPos, Vec2 const& endPos, Vec2 const& discCenter, float discRadius );
/// Ray cast to hit a 2D line segment, returns whether the ray hits the line segment
bool RayCastVsLineSegment2D( RayCastResult2D& out_rayCastRes, Vec2 const& startPos, Vec2 const& forwardNormal, float maxDist, Vec2 const& lineStartPos, Vec2 const& lineEndPos );
/// Ray cast to hit an AABB 2D, returns whether the ray hits the AABB2
bool RayCastVsAABB2D( RayCastResult2D& out_rayCastRes, Vec2 const& startPos, Vec2 const& forwardNormal, float maxDist, AABB2 const& aabb2 );
/// Ray cast to hit an AABB 2D, returns only the result of the ray hitting the AABB2
bool RayCastVsAABB2DResultOnly( Vec2 const& startPos, Vec2 const& forwardNormal, float maxDist, AABB2 const& aabb2 );
/// Ray cast to hit an convex hull 2D, returns whether the ray hits the convex hull
bool RayCastVsConvexHull2D( RayCastResult2D& out_rayCastRes, Vec2 const& startPos, Vec2 const& forwardNormal, float maxDist, ConvexHull2 const& convexHull );
/// Ray cast to hit a 3D vertical cylinder, returns whether the ray hits the cylinder
bool RayCastVsCylinderZ3D( RayCastResult3D& out_rayCastRes, Vec3 const& startPos, Vec3 const& forwardNormal, float maxDist, Vec2 const& cylinderCenter, float minZ, float maxZ, float radius );
/// Ray cast to hit a 3D vertical cylinder, returns whether the ray hits the cylinder
bool RayCastVsCylinderZ3D( RayCastResult3D& out_rayCastRes, Ray3D const& ray3D, Vec2 const& cylinderCenter, float minZ, float maxZ, float radius );
/// Ray cast to hit a Sphere, returns whether the ray hits the sphere
bool RayCastVsSphere3D( RayCastResult3D& out_rayCastRes, Vec3 const& startPos, Vec3 const& forwardNormal, float maxDist, Vec3 const& center, float radius );
/// Ray cast to hit an AABB3D, returns whether the ray hits the AABB3D
bool RayCastVsAABB3D( RayCastResult3D& out_rayCastRes, Vec3 const& startPos, Vec3 const& forwardNormal, float maxDist, AABB3 const& aabb3 );
/// Ray cast to hit an OBB3D, returns whether the ray hits the OBB3D
bool RayCastVsOBB3D( RayCastResult3D& out_rayCastRes, Vec3 const& startPos, Vec3 const& forwardNormal, float maxDist, OBB3 const& obb3 );
///  Ray cast to hit an Plane3D, returns whether the ray hits the 3D Plane
bool RayCastVsPlane3D( RayCastResult3D& out_rayCastRes, Vec3 const& startPos, Vec3 const& forwardNormal, float maxDist, Plane3 const& plane );