#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/IntVec3.hpp"
#include <math.h>

void IntVec3::operator/=( int const uniformDivisor )
{
	x /= uniformDivisor;
	y /= uniformDivisor;
	z /= uniformDivisor;
}

const IntVec3 IntVec3::operator*( int uniformScale ) const
{
	return IntVec3( x * uniformScale, y * uniformScale, z * uniformScale );
}

const IntVec3 IntVec3::operator*( IntVec3 const& vecToMultiply ) const
{
	return IntVec3( x * vecToMultiply.x, y * vecToMultiply.y, z * vecToMultiply.z );
}

const IntVec3 IntVec3::operator/( int inverseScale ) const
{
	return IntVec3( x / inverseScale, y / inverseScale, z / inverseScale );
}

void IntVec3::operator*=( int const uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
}

void IntVec3::operator-=( IntVec3 const& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}

void IntVec3::operator+=( IntVec3 const& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}

IntVec3 const IntVec3::ZERO = IntVec3( 0, 0, 0 );

IntVec3::IntVec3( const IntVec3& copyFrom )
	:x( copyFrom.x )
	,y( copyFrom.y )
	,z( copyFrom.z )
{

}

IntVec3::IntVec3( int initialX, int initialY, int initialZ )
	:x( initialX )
	,y( initialY )
	,z( initialZ )
{

}

float IntVec3::GetLength() const
{
	return sqrtf( (float)x * (float)x + (float)y * (float)y + (float)z * (float)z );
}

int IntVec3::GetTaxicabLength() const
{
	return abs( x ) + abs( y ) + abs( z );
}

int IntVec3::GetLengthSquared() const
{
	return x * x + y * y + z * z;
}

bool IntVec3::operator==( const IntVec3& compare ) const
{
	return x == compare.x && y == compare.y && z == compare.z;
}

const IntVec3 IntVec3::operator+( const IntVec3& vecToAdd ) const
{
	return IntVec3( x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z );
}

const IntVec3 IntVec3::operator-( const IntVec3& vecToSub ) const
{
	return IntVec3( x - vecToSub.x, y - vecToSub.y, z - vecToSub.z );
}

const IntVec3 IntVec3::operator-() const
{
	return IntVec3( -x, -y, -z );
}

bool IntVec3::operator!=( IntVec3 const& compare ) const
{
	return x != compare.x || y != compare.y || z != compare.z;
}

void IntVec3::operator=( const IntVec3& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}


IntVec3 const operator*( int uniformScale, IntVec3 const& vecToScale )
{
	return IntVec3( vecToScale.x * uniformScale, vecToScale.y * uniformScale, vecToScale.z * uniformScale );
}