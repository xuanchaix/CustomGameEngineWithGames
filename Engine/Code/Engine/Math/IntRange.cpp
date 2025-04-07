#include "Engine/Math/IntRange.hpp"

IntRange::IntRange()
	:m_min(0),
	m_max(0)
{

}

IntRange::IntRange( int min, int max )
	:m_max(max),
	m_min(min)
{

}

bool IntRange::IsOnRange( float num ) const
{
	return num <= m_max && num >= m_min;
}

bool IntRange::IsOverlappingWith( IntRange const& comparedIntRange )
{
	return !(comparedIntRange.m_min > m_max || comparedIntRange.m_max < m_min);
}

bool IntRange::operator!=( IntRange const& compare ) const
{
	return compare.m_max != m_max && compare.m_min != m_min;
}

bool IntRange::operator==( IntRange const& compare ) const
{
	return compare.m_max == m_max && compare.m_min == m_min;
}

void IntRange::operator=( IntRange const& copyFrom )
{
	m_max = copyFrom.m_max;
	m_min = copyFrom.m_min;
}
