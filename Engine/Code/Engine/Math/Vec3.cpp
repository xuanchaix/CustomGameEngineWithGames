#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/IntVec3.hpp"
#include <math.h>

Vec3 const Vec3::Zero = Vec3( 0, 0, 0 );
Vec3 const Vec3::One = Vec3( 1, 1, 1 );
Vec3 const Vec3::XAxis = Vec3( 1, 0, 0 );
Vec3 const Vec3::YAxis = Vec3( 0, 1, 0 );
Vec3 const Vec3::ZAxis = Vec3( 0, 0, 1 );
Vec3 const Vec3::Forward = Vec3( 1, 0, 0 );
Vec3 const Vec3::Backward = Vec3( -1, 0, 0 );
Vec3 const Vec3::Left = Vec3( 0, 1, 0 );
Vec3 const Vec3::Right = Vec3( 0, -1, 0 );
Vec3 const Vec3::Up = Vec3( 0, 0, 1 );
Vec3 const Vec3::Down = Vec3( 0, 0, -1 );

Vec3::Vec3(const float x, const float y, const float z)
	:x(x)
	,y(y)
	,z(z)
{

}

Vec3::Vec3( const Vec3& copyFrom )
	:x(copyFrom.x)
	,y(copyFrom.y)
	,z(copyFrom.z)
{

}

Vec3::Vec3( Vec2 const& copyFrom, float z )
	:x( copyFrom.x )
	,y( copyFrom.y )
	,z( z )
{

}

Vec3::Vec3( IntVec3 const& copyFrom )
	:x((float)copyFrom.x)
	,y((float)copyFrom.y)
	,z((float)copyFrom.z)
{

}

bool Vec3::SetFromText( char const* text )
{
	Strings strs;
	strs.reserve( 4 );
	int numOfStrings = SplitStringOnDelimiter( strs, text, ',' );
	if (numOfStrings != 3) {
		return false;
	}
	try{
		x = stof( strs[0] );
		y = stof( strs[1] );
		z = stof( strs[2] );
	}
	catch (std::exception const& e)
	{
		UNUSED( e );
		return false;
	}
	return true;
}

Vec3 const Vec3::MakeFromPolarRadians( float latitudeRadians, float longitudeRadians, float length /*= 1.f */ )
{
	return Vec3(	length * CosRadians( latitudeRadians ) * CosRadians( longitudeRadians ),
					length * CosRadians( latitudeRadians ) * SinRadians( longitudeRadians ),
					-length * SinRadians( latitudeRadians ) );
}

Vec3 const Vec3::MakeFromPolarDegrees( float latitudeDegrees, float longitudeDegrees, float length /*= 1.f */ )
{
	return Vec3(	length * CosDegrees( latitudeDegrees ) * CosDegrees( longitudeDegrees ), 
					length * CosDegrees( latitudeDegrees ) * SinDegrees( longitudeDegrees ), 
					-length * SinDegrees( latitudeDegrees ) );
}

float Vec3::GetLength() const
{
	return sqrtf( x * x + y * y + z * z );
}

float Vec3::GetLengthXY() const
{
	return sqrtf( x * x + y * y );
}

float Vec3::GetLengthSquared() const
{
	return x * x + y * y + z * z;
}

float Vec3::GetLengthXYSquared() const
{
	return x * x + y * y;
}

float Vec3::GetAngleAboutZRadians() const
{
	return Atan2Radians( y, x );
}

float Vec3::GetAngleAboutZDegrees() const
{
	return Atan2Degrees( y, x );
}

Vec3 const Vec3::GetRotatedAboutZRadians( float deltaRadians ) const
{
	Vec3 v = Vec3( *this );
	TransformPositionXY3D( v, 1.f, ConvertRadiansToDegrees( deltaRadians ), Vec2( 0, 0 ) );
	return v;
}

Vec3 const Vec3::GetRotatedAboutZDegrees( float deltaDegrees ) const
{
	Vec3 v = Vec3( *this );
	TransformPositionXY3D( v, 1.f, deltaDegrees, Vec2( 0, 0 ) );
	return v;
}

Vec3 const Vec3::GetClamped( float maxLength ) const
{
	float length = GetLength();
	if (length > maxLength && maxLength >= 0 && length > 0) {
		/*float phi = GetAngleAboutZRadians();
		float theta = acosf( z / length );
		return Vec3(maxLength * sinf(theta) * cosf(phi), maxLength * sinf(theta) * sinf(phi), maxLength * cosf(theta));*/
		return Vec3( x, y, z ) * maxLength / length;
	}
	return Vec3(x, y, z);
}

Vec3 const Vec3::GetNormalized() const
{
	if (x != 0.f || y != 0.f || z != 0.f) {
		float r = GetLength();
		return Vec3( x / r, y / r, z / r );
	}
	else {
		return Vec3( 0, 0, 0 );
	}
}

EulerAngles Vec3::GetOrientationEulerAngles() const
{
	EulerAngles eulerAngles;
	eulerAngles.m_yawDegrees = Atan2Degrees( y, x );
	eulerAngles.m_pitchDegrees = -Atan2Degrees( z, sqrtf( x * x + y * y ) );
	eulerAngles.m_rollDegrees = 0.f;
	return eulerAngles;
}

void Vec3::Normalize()
{
	float length = GetLength();
	if (length == 0.f) {
		return;
	}
	float reversedLength = 1 / length;
	x *= reversedLength;
	y *= reversedLength;
	z *= reversedLength;
}

float Vec3::NormalizeAndGetPreviousLength()
{
	float length = GetLength();
	if (length == 0.f) {
		return 0.f;
	}
	float reversedLength = 1 / length;
	x *= reversedLength;
	y *= reversedLength;
	z *= reversedLength;
	return length;
}

bool Vec3::operator==( Vec3 const& compare ) const
{
	if (x == compare.x && y == compare.y && z == compare.z) {
		return true;
	}
	return false;
}

bool Vec3::operator!=( Vec3 const& compare ) const
{
	if (x == compare.x && y == compare.y && z == compare.z) {
		return false;
	}
	return true;
}

Vec3 const Vec3::operator+( Vec3 const& VecToAdd ) const
{
	return Vec3( x + VecToAdd.x, y + VecToAdd.y, z + VecToAdd.z );
}

Vec3 const Vec3::operator-( Vec3 const& vecToSubtract ) const
{
	return Vec3( x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z );
}

Vec3 const Vec3::operator-() const
{
	return Vec3( -x, -y, -z );
}

void Vec3::operator=( Vec3 const& copyfrom )
{
	x = copyfrom.x;
	y = copyfrom.y;
	z = copyfrom.z;
}

void Vec3::operator=( Vec2 const& copyfrom )
{
	x = copyfrom.x;
	y = copyfrom.y;
	z = 0.f;
}

Vec3 const Vec3::operator*( float uniformScale ) const
{
	return Vec3( x * uniformScale, y * uniformScale, z * uniformScale );
}

Vec3 const Vec3::operator*( const Vec3& vecToMultiply ) const
{
	return Vec3( x * vecToMultiply.x, y * vecToMultiply.y, z * vecToMultiply.z );
}

Vec3 const Vec3::operator/( float inverseScale ) const
{
	return Vec3( x / inverseScale, y / inverseScale, z / inverseScale );
}

void Vec3::operator+=( Vec3 const& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}

void Vec3::operator*=( float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
}

void Vec3::operator-=( Vec3 const& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}

void Vec3::operator/=( float uniformDivisor )
{
	x /= uniformDivisor;
	y /= uniformDivisor;
	z /= uniformDivisor;
}

Vec3 const operator*( float uniformScale, Vec3 const& vecToScale ) 
{
	return Vec3( vecToScale.x * uniformScale, vecToScale.y * uniformScale, vecToScale.z * uniformScale );
}
