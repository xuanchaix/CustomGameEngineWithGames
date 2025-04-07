#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

OBB2::OBB2()
	:m_center(Vec2(0, 0)),
	m_halfDimensions(Vec2(1, 1)),
	m_iBasisNormal(Vec2(1, 0))
{

}

OBB2::OBB2( Vec2 const& center, Vec2 const& iBasisNormal, Vec2 const& halfDimensions )
	:m_center(center),
	m_halfDimensions(halfDimensions),
	m_iBasisNormal(iBasisNormal)
{

}

OBB2::~OBB2()
{

}

Vec2 const OBB2::GetLocalPosFromWorldPos( Vec2 const& worldPos ) const
{
	return (worldPos - m_center).GetRotatedNewBasis( GetInversedOrthonormaliBasis( m_iBasisNormal ) );
}

bool OBB2::IsPointInside( Vec2 const& point ) const
{
	Vec2 localPoint = GetLocalPosFromWorldPos( point );
	return localPoint.x < m_halfDimensions.x && localPoint.x > -m_halfDimensions.x && localPoint.y < m_halfDimensions.y && localPoint.y > -m_halfDimensions.y;
}

Vec2 const OBB2::GetNearestPoint( Vec2 const& referencePosition ) const
{
	Vec2 localPoint = GetLocalPosFromWorldPos( referencePosition );
	Vec2 localNearestPoint = Vec2( GetClamped( localPoint.x, -m_halfDimensions.x, m_halfDimensions.x ), GetClamped( localPoint.y, -m_halfDimensions.y, m_halfDimensions.y ) );
	return localNearestPoint.GetRotatedNewBasis( m_iBasisNormal ) + m_center;
}

Vec2 OBB2::GetRandomPointInside() const
{
	float i = g_engineRNG->RollRandomFloatInRange( -m_halfDimensions.x, m_halfDimensions.x );
	float j = g_engineRNG->RollRandomFloatInRange( -m_halfDimensions.y, m_halfDimensions.y );
	return m_center + m_iBasisNormal * i + m_iBasisNormal.GetRotated90Degrees() * j;
}

void OBB2::Translate( Vec2 const& translationToApply )
{
	m_center += translationToApply;
}

void OBB2::SetCenter( Vec2 const& newCenter )
{
	m_center = newCenter;
}

void OBB2::SetDimensions( Vec2 const& newHalfDimensions )
{
	m_halfDimensions = newHalfDimensions;
}

void OBB2::RotateAboutCenter( float rotationDeltaDegrees )
{
	m_iBasisNormal.RotateDegrees( rotationDeltaDegrees );
}
