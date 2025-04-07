#include "Engine/Math/Plane.hpp"
#include "Engine/Math/MathUtils.hpp"

Plane3::Plane3( Vec3 const& normal, float distanceFromOrigin )
	:m_normal(normal)
	,m_distanceFromOrigin(distanceFromOrigin)
{

}

Plane3::Plane3()
{

}

Vec3 Plane3::GetOriginPoint() const
{
	return m_normal * m_distanceFromOrigin;
}

float Plane3::GetAltitudeOfPoint( Vec3 const& refPoint ) const
{
	return DotProduct3D( m_normal, refPoint ) - m_distanceFromOrigin;
}

Vec3 Plane3::GetNearestPoint( Vec3 const& refPoint ) const
{
	return refPoint - m_normal * GetAltitudeOfPoint( refPoint );
}

void Plane3::Translate( Vec3 const& translationToApply )
{
	// translate do not change the normal and all points on the plane translates
	Vec3 translatedOrigin = GetOriginPoint() + translationToApply;
	m_distanceFromOrigin = DotProduct3D( translatedOrigin, m_normal );
}

Plane2::Plane2()
{

}

Plane2::Plane2( Vec2 const& normal, Vec2 const& refPosOnPlane )
	:m_normal(normal)
	,m_distanceFromOrigin(DotProduct2D(normal, refPosOnPlane))
{

}

Vec2 Plane2::GetOriginPoint() const
{
	return m_normal * m_distanceFromOrigin;
}

float Plane2::GetAltitudeOfPoint( Vec2 const& refPoint ) const
{
	return DotProduct2D( m_normal, refPoint ) - m_distanceFromOrigin;
}

Vec2 Plane2::GetNearestPoint( Vec2 const& refPoint ) const
{
	return refPoint - m_normal * GetAltitudeOfPoint( refPoint );
}
