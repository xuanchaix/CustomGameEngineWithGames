#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

FloatRange const FloatRange::ONE = FloatRange( 1.f, 1.f );
FloatRange const FloatRange::ZERO = FloatRange( 0.f, 0.f );
FloatRange const FloatRange::ZERO_TO_ONE = FloatRange( 0.f, 1.f );


FloatRange::FloatRange()
	:m_max(0.f),
	m_min(0.f)
{

}

FloatRange::FloatRange( float min, float max )
	:m_max(max),
	m_min(min)
{
}

bool FloatRange::SetFromText( char const* text )
{
	Strings strs;
	strs.reserve( 4 );
	int numOfStrings = SplitStringOnDelimiter( strs, text, '~' );
	if (numOfStrings != 2) {
		strs.clear();
		numOfStrings = SplitStringOnDelimiter( strs, text, '-' );
		if (numOfStrings != 2) {
			return false;
		}
	}
	try{
		m_min = stof( strs[0] );
		m_max = stof( strs[1] );
	}
	catch (std::exception const& e)
	{
		UNUSED( e );
		return false;
	}
	return true;
}

bool FloatRange::IsOnRange( float num ) const
{
	return num <= m_max&& num >= m_min;
}

bool FloatRange::IsOverlappingWith( FloatRange const& comparedFloatRange )
{
	return !(comparedFloatRange.m_min > m_max || comparedFloatRange.m_max < m_min);
}

bool FloatRange::operator!=( FloatRange const& compare ) const
{
	return compare.m_max != m_max || compare.m_min != m_min;
}

bool FloatRange::operator==( FloatRange const& compare ) const
{
	return compare.m_max == m_max && compare.m_min == m_min;
}

void FloatRange::operator=( FloatRange const& copyFrom )
{
	m_max = copyFrom.m_max;
	m_min = copyFrom.m_min;
}
