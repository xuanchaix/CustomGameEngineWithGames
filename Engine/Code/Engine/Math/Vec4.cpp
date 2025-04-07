#include "Engine/Math/Vec4.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec3.hpp"

Vec4::Vec4( const Vec4& copyFrom )
	:x(copyFrom.x)
	,y(copyFrom.y)
	,z(copyFrom.z)
	,w(copyFrom.w)
{
}

Vec4::Vec4( float initialX, float initialY, float initialZ, float initialW )
	:x( initialX )
	,y( initialY )
	,z( initialZ )
	,w( initialW )
{
}

Vec4::Vec4( Vec3 const& vector3, float initialW )
	:x( vector3.x )
	, y( vector3.y )
	, z( vector3.z )
	, w( initialW )
{
}

bool Vec4::SetFromText( char const* text )
{
	Strings strs;
	strs.reserve( 4 );
	int numOfStrings = SplitStringOnDelimiter( strs, text, ',' );
	if (numOfStrings != 4) {
		return false;
	}
	try{
		x = stof( strs[0] );
		y = stof( strs[1] );
		z = stof( strs[2] );
		w = stof( strs[3] );
	}
	catch (std::exception const& e)
	{
		UNUSED( e );
		return false;
	}
	return true;
}

Vec4 const Vec4::operator+( Vec4 const& VecToAdd ) const
{
	return Vec4( x + VecToAdd.x, y + VecToAdd.y, z + VecToAdd.z, w + VecToAdd.w );
}

Vec4 const Vec4::operator-( Vec4 const& vecToSubtract ) const
{
	return Vec4( x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z, w - vecToSubtract.w );
}

Vec4 const Vec4::operator-() const
{
	return Vec4( -x, -y, -z, -w );
}

void Vec4::operator-=( Vec4 const& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
	w -= vecToSubtract.w;
}

void Vec4::operator/=( float uniformDivisor )
{
	x /= uniformDivisor;
	y /= uniformDivisor;
	z /= uniformDivisor;
	w /= uniformDivisor;
}

void Vec4::operator=( Vec4 const& copyfrom )
{
	x = copyfrom.x;
	y = copyfrom.y;
	z = copyfrom.z;
	w = copyfrom.w;
}

void Vec4::operator*=( float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
	w *= uniformScale;
}

void Vec4::operator+=( Vec4 const& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
	w += vecToAdd.w;
}

Vec4 const Vec4::operator*( float uniformScale ) const
{
	return Vec4( x * uniformScale, y * uniformScale, z * uniformScale, w * uniformScale );
}

Vec4 const Vec4::operator*( const Vec4& vecToMultiply ) const
{
	return Vec4( x * vecToMultiply.x, y * vecToMultiply.y, z * vecToMultiply.z, w * vecToMultiply.w );
}

Vec4 const Vec4::operator/( float inverseScale ) const
{
	return Vec4( x / inverseScale, y / inverseScale, z / inverseScale, w / inverseScale );
}

bool Vec4::operator!=( Vec4 const& compare ) const
{
	return x != compare.x || y != compare.y || z != compare.z || w != compare.w;
}

bool Vec4::operator==( Vec4 const& compare ) const
{
	return x == compare.x && y == compare.y && z == compare.z && w == compare.w;
}
