#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/IntVec3.hpp"
#include <math.h>

bool IntVec2::SetFromText( char const* text )
{
	Strings strs;
	strs.reserve( 4 );
	int numOfStrings = SplitStringOnDelimiter( strs, text, ',' );
	if (numOfStrings != 2) {
		return false;
	}
	try{
		x = stoi( strs[0] );
		y = stoi( strs[1] );
	}
	catch (std::exception const& e)
	{
		UNUSED( e );
		return false;
	}
	return true;
}

void IntVec2::operator/=( int const uniformDivisor )
{
	x /= uniformDivisor;
	y /= uniformDivisor;
}

const IntVec2 IntVec2::operator*( int uniformScale ) const
{
	return IntVec2( x * uniformScale, y * uniformScale );
}

const IntVec2 IntVec2::operator*( IntVec2 const& vecToMultiply ) const
{
	return IntVec2( x * vecToMultiply.x, y * vecToMultiply.y );
}

const IntVec2 IntVec2::operator/( int inverseScale ) const
{
	return IntVec2( x / inverseScale, y / inverseScale );
}

void IntVec2::operator*=( int const uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
}

void IntVec2::operator-=( IntVec2 const& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}

void IntVec2::operator+=( IntVec2 const& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}

IntVec2 const IntVec2::ZERO = IntVec2( 0, 0 );

IntVec2::IntVec2( const IntVec2& copyFrom )
		:x(copyFrom.x)
		,y(copyFrom.y)
{

}

IntVec2::IntVec2( int initialX, int initialY )
		:x(initialX)
		,y(initialY)
{

}

IntVec2::IntVec2( IntVec3 const& copyFrom )
	:x( copyFrom.x )
	, y( copyFrom.y )
{

}

float IntVec2::GetLength() const
{
	return sqrtf( (float)x * (float)x + (float)y * (float)y );
}

int IntVec2::GetTaxicabLength() const
{
	return abs( x ) + abs( y );
}

int IntVec2::GetLengthSquared() const
{
	return x * x + y * y;
}

float IntVec2::GetOrientationRadians() const
{
	return Atan2Radians( (float)y, (float)x );
}

float IntVec2::GetOrientationDegrees() const
{
	return Atan2Degrees( (float)y, (float)x );
}

IntVec2 const IntVec2::GetRotated90Degrees() const
{
	return IntVec2( -y, x );
}

IntVec2 const IntVec2::GetRotatedMinus90Degrees() const
{
	return IntVec2( y, -x );
}

void IntVec2::Rotate90Degrees()
{
	int temp_x = x;
	x = -y;
	y = temp_x;
}

void IntVec2::RotateMinus90Degrees()
{
	
	int temp_x = x;
	x = y;
	y = -temp_x;
}

bool IntVec2::operator==( const IntVec2& compare ) const
{
	return x == compare.x && y == compare.y;
}

const IntVec2 IntVec2::operator+( const IntVec2& vecToAdd ) const
{
	return IntVec2( x + vecToAdd.x, y + vecToAdd.y );
}

const IntVec2 IntVec2::operator-( const IntVec2& vecToSub ) const
{
	return IntVec2( x - vecToSub.x, y - vecToSub.y );
}

const IntVec2 IntVec2::operator-() const
{
	return IntVec2( -x, -y );
}

bool IntVec2::operator!=( IntVec2 const& compare ) const
{
	return x != compare.x || y != compare.y;
}

void IntVec2::operator=(const IntVec2& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
}


IntVec2 const operator*( int uniformScale, IntVec2 const& vecToScale )
{
	return IntVec2( vecToScale.x * uniformScale, vecToScale.y * uniformScale );
}