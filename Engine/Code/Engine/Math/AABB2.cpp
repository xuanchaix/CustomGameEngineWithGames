#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"

AABB2 const AABB2::IDENTITY = AABB2( Vec2( 0, 0 ), Vec2( 1, 1 ) );

AABB2::AABB2( AABB2 const& copyFrom )
	:m_mins(copyFrom.m_mins)
	,m_maxs(copyFrom.m_maxs)
{

}

AABB2::AABB2( float minX, float minY, float maxX, float maxY )
	:m_mins(Vec2(minX, minY))
	,m_maxs(Vec2(maxX, maxY))
{

}

AABB2::AABB2( Vec2 const& mins, Vec2 const& maxs )
	:m_mins(mins)
	,m_maxs(maxs)
{

}

bool AABB2::IsPointInside( Vec2 const& point ) const
{
	return point.x > m_mins.x && point.y > m_mins.y && point.x < m_maxs.x && point.y < m_maxs.y;
}

bool AABB2::IsPointInsideOrOn( Vec2 const& point ) const
{
	return point.x >= m_mins.x && point.y >= m_mins.y && point.x <= m_maxs.x && point.y <= m_maxs.y;
}

Vec2 const AABB2::GetCenter() const
{
	return (m_mins + m_maxs) * 0.5f;
}

Vec2 const AABB2::GetDimensions() const
{
	return m_maxs - m_mins;
}

Vec2 const AABB2::GetNearestPoint( Vec2 const& referencePosition ) const
{
	return Vec2( GetClamped( referencePosition.x, m_mins.x, m_maxs.x ), GetClamped( referencePosition.y, m_mins.y, m_maxs.y ) );
}

Vec2 const AABB2::GetPointAtUV( Vec2 const& uv ) const
{
	return Vec2(m_mins.x + uv.x * (m_maxs.x - m_mins.x), m_mins.y + uv.y * (m_maxs.y - m_mins.y));
}

Vec2 const AABB2::GetUVForPoint( Vec2 const& point ) const
{
	return Vec2((point.x - m_mins.x) / (m_maxs.x - m_mins.x), (point.y - m_mins.y) / (m_maxs.y - m_mins.y));
}

Vec2 AABB2::GetRandomPointInside() const
{
	return GetRandomPointInAABB2D( *this );
}

void AABB2::Translate( Vec2 const& translationToApply )
{
	m_mins += translationToApply;
	m_maxs += translationToApply;
}

void AABB2::SetCenter( Vec2 const& newCenter )
{
	Vec2 translation = newCenter - GetCenter();
	m_mins += translation;
	m_maxs += translation;
}

void AABB2::SetDimensions( Vec2 const& newDimensions )
{
	Vec2 center = GetCenter();
	m_mins = center - newDimensions * 0.5f;
	m_maxs = center + newDimensions * 0.5f;
}

void AABB2::StretchToIncludePoint( Vec2 const& point )
{
	if (m_mins.x > point.x) {
		m_mins.x = point.x;
	}
	if (m_mins.y > point.y) {
		m_mins.y = point.y;
	}
	if (m_maxs.x < point.x) {
		m_maxs.x = point.x;
	}
	if (m_maxs.y < point.y) {
		m_maxs.y = point.y;
	}
}
