#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec2.hpp"

struct Plane2 {
public:
	Plane2();
	Plane2( Vec2 const& normal, Vec2 const& refPosOnPlane );
	/// Get the point on plane nearest to origin(0,0,0)
	Vec2 GetOriginPoint() const;
	/// 
	float GetAltitudeOfPoint( Vec2 const& refPoint ) const;
	/// 
	Vec2 GetNearestPoint( Vec2 const& refPoint ) const;
	Vec2 m_normal;
	float m_distanceFromOrigin;
};

struct Plane3 {
public:
	Plane3();
	Plane3( Vec3 const& normal, float distanceFromOrigin );

	/// Get the point on plane nearest to origin(0,0,0)
	Vec3 GetOriginPoint() const;
	/// 
	float GetAltitudeOfPoint( Vec3 const& refPoint ) const;
	/// 
	Vec3 GetNearestPoint( Vec3 const& refPoint ) const;
	/// 
	void Translate( Vec3 const& translationToApply );
public:
	Vec3 m_normal = Vec3( 0.f, 0.f, 1.f );
	float m_distanceFromOrigin = 0.f;
};