#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/MathUtils.hpp"

AABB3::AABB3( AABB3 const& copyFrom )
	:m_mins( copyFrom.m_mins )
	, m_maxs( copyFrom.m_maxs )
{

}

AABB3::AABB3( Vec3 const& mins, Vec3 const& maxs )
	:m_mins(mins), m_maxs(maxs)
{

}

AABB3::AABB3( float minX, float minY, float minZ, float maxX, float maxY, float maxZ )
	: m_mins( Vec3(minX, minY, minZ) ), m_maxs( Vec3(maxX, maxY, maxZ ))
{

}

bool AABB3::IsPointInside( Vec3 const& point ) const
{
	return point.x > m_mins.x && point.y > m_mins.y && point.x < m_maxs.x&& point.y < m_maxs.y && point.z > m_mins.z && point.z < m_maxs.z;
}

Vec3 AABB3::GetNearestPoint( Vec3 const& referencePosition ) const
{
	Vec3 retValue;
	retValue.x = GetClamped( referencePosition.x, m_mins.x, m_maxs.x );
	retValue.y = GetClamped( referencePosition.y, m_mins.y, m_maxs.y );
	retValue.z = GetClamped( referencePosition.z, m_mins.z, m_maxs.z );
	return retValue;
}
