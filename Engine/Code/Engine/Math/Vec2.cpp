#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include <Math.h>
//#include "Engine/Core/EngineCommon.hpp"

Vec2 const Vec2::Zero = Vec2( 0, 0 );
Vec2 const Vec2::One = Vec2( 1, 1 );
Vec2 const Vec2::XAxis = Vec2( 1, 0 );
Vec2 const Vec2::YAxis = Vec2( 0, 1 );
Vec2 const Vec2::Unit45Degree = Vec2( INV_SQRT_2, INV_SQRT_2 );

//-----------------------------------------------------------------------------------------------
Vec2::Vec2( const Vec2& copy )
	: x( copy.x )
	, y( copy.y )
{
}


//-----------------------------------------------------------------------------------------------
Vec2::Vec2( float initialX, float initialY )
	: x( initialX )
	, y( initialY )
{
}


Vec2::Vec2( IntVec2 const& copyFrom )
	:x((float)copyFrom.x)
	,y((float)copyFrom.y)
{
}

Vec2::Vec2( Vec3 const& copyFrom )
	:x(copyFrom.x)
	,y(copyFrom.y)
{

}

bool Vec2::SetFromText( char const* text )
{
	Strings strs;
	strs.reserve( 4 );
	int numOfStrings = SplitStringOnDelimiter( strs, text, ',' );
	if (numOfStrings != 2) {
		return false;
	}
	try{
		x = stof( strs[0] );
		y = stof( strs[1] );
	}
	catch (std::exception const& e)
	{
		UNUSED( e );
		return false;
	}
	return true;
}

Vec2 const Vec2::MakeFromPolarRadians( float orientationRadians, float length /* = 1.f */ )
{
	return Vec2( CosRadians( orientationRadians ) * length, SinRadians( orientationRadians ) * length );
}

Vec2 const Vec2::MakeFromPolarDegrees( float orientationDegrees, float length /* = 1.f */ )
{
	return Vec2( CosDegrees( orientationDegrees ) * length, SinDegrees( orientationDegrees ) * length );
}

//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator + ( const Vec2& vecToAdd ) const
{
	return Vec2( x + vecToAdd.x, y + vecToAdd.y );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-( const Vec2& vecToSubtract ) const
{
	return Vec2( x - vecToSubtract.x, y - vecToSubtract.y );
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-() const
{
	return Vec2( -x, -y );
}


void Vec2::Normalize()
{
	//GUARANTEE_OR_DIE( x != 0.f || y != 0.f, "Cannot normalize 0 vector!" );
	if (x == 0.f && y == 0.f) {
		return;
	}
	float length = sqrtf( x * x + y * y );
	float scale = 1.f / length;
	x *= scale;
	y *= scale;
}

float Vec2::NormalizeAndGetPreviousLength()
{
	//GUARANTEE_OR_DIE( x != 0.f || y != 0.f, "Cannot normalize 0 vector!" );
	if (x == 0.f && y == 0.f) {
		return 0.f;
	}
	float length = sqrtf( x * x + y * y );
	float scale = 1.f / length;
	x *= scale;
	y *= scale;
	return length;
}

void Vec2::Reflect( Vec2 const NormalizedReflectNormal )
{
	Vec2 res = *this - 2.f * (DotProduct2D( NormalizedReflectNormal, *this ) * NormalizedReflectNormal);
	x = res.x;
	y = res.y;
}

float Vec2::GetLength() const
{
	return sqrtf( x * x + y * y );
}

float Vec2::GetLengthSquared() const
{
	return x * x + y * y;
}

float Vec2::GetOrientationRadians() const
{
	return Atan2Radians( y, x );
}

float Vec2::GetOrientationDegrees() const
{
	return Atan2Degrees( y, x );
}

Vec2 const Vec2::GetRotated90Degrees() const
{
	//Vec2 v = Vec2( x, y );
	//TransformPosition2D( v, 1.f, 90.f, Vec2( 0, 0 ) );
	return Vec2( -y, x );
}

Vec2 const Vec2::GetRotatedMinus90Degrees() const
{
	//Vec2 v = Vec2( x, y );
	//TransformPosition2D( v, 1.f, -90.f, Vec2( 0, 0 ) );
	return Vec2( y, -x );
}

Vec2 const Vec2::GetRotatedRadians( float deltaRadians ) const
{
	return GetRotatedNewBasis( Vec2( CosRadians( deltaRadians ), SinRadians( deltaRadians ) ) );
}

Vec2 const Vec2::GetRotatedDegrees( float deltaDegrees ) const
{
	return GetRotatedNewBasis( Vec2( CosDegrees( deltaDegrees ), SinDegrees( deltaDegrees ) ) );
}

Vec2 const Vec2::GetRotatedNewBasis( Vec2 const& iBasisNormal ) const
{
	Vec2 jBasisNormal = Vec2( -iBasisNormal.y, iBasisNormal.x );
	return x * iBasisNormal + y * jBasisNormal;
}

Vec2 const Vec2::GetClamped( float maxLength ) const
{
	float len = GetLength();
	if (len > maxLength && maxLength >= 0.f && len != 0.f) {
		return Vec2( x, y ) * maxLength / len;
	}
	return Vec2( x, y );
}

Vec2 const Vec2::GetNormalized() const
{
	//GUARANTEE_OR_DIE( x != 0.f || y != 0.f, "Cannot normalize 0 vector!" );
	if (x == 0.f && y == 0.f) {
		return Vec2( 0.f, 0.f );
	}
	float length = sqrtf( x * x + y * y );
	float scale = 1.f / length;
	return Vec2( x * scale, y * scale );
}

Vec2 const Vec2::GetReflected( Vec2 const NormalizedReflectNormal ) const
{
	return *this - 2 * (DotProduct2D( NormalizedReflectNormal, *this ) * NormalizedReflectNormal);
}

void Vec2::SetOrientationRadians( float newOrientationRadians )
{
	float length = GetLength();
	x = length * CosRadians( newOrientationRadians );
	y = length * SinRadians( newOrientationRadians );
}

void Vec2::SetOrientationDegrees( float newOrientationDegrees )
{
	float length = GetLength();
	x = length * CosDegrees( newOrientationDegrees );
	y = length * SinDegrees( newOrientationDegrees );
}

void Vec2::SetPolarRadians( float newOrientationRadians, float newLength )
{
	x = newLength * CosRadians( newOrientationRadians );
	y = newLength * SinRadians( newOrientationRadians );
}

void Vec2::SetPolarDegrees( float newOrientationDegrees, float newLength )
{
	x = newLength * CosDegrees( newOrientationDegrees );
	y = newLength * SinDegrees( newOrientationDegrees );
}

void Vec2::Rotate90Degrees()
{
	float temp_x = x;
	x = -y;
	y = temp_x;
	//TransformPosition2D( *this, 1.f, 90.f, Vec2( 0, 0 ) );
}

void Vec2::RotateMinus90Degrees()
{
	float temp_x = x;
	x = y;
	y = -temp_x;
	//TransformPosition2D( *this, 1.f, -90.f, Vec2( 0, 0 ) );
}

void Vec2::RotateRadians( float deltaRadians )
{
	RotateNewBasis( Vec2( CosRadians( deltaRadians ), SinRadians( deltaRadians ) ) );
}

void Vec2::RotateDegrees( float deltaDegrees )
{
	RotateNewBasis( Vec2( CosDegrees( deltaDegrees ), SinDegrees( deltaDegrees ) ) );
}

void Vec2::RotateNewBasis( Vec2 const& iBasisNormal )
{
	Vec2 jBasisNormal = Vec2( -iBasisNormal.y, iBasisNormal.x );
	float tempx = x;
	x = iBasisNormal.x * x + jBasisNormal.x * y;
	y = iBasisNormal.y * tempx + jBasisNormal.y * y;
}

void Vec2::SetLength( float newLength )
{
	float radians = GetOrientationRadians();
	x = newLength * CosRadians( radians );
	y = newLength * SinRadians( radians );
}

void Vec2::ClampLength( float maxLength )
{
	float len = GetLength();
	if (len > maxLength && maxLength >= 0.f && len != 0.f) {
		x = x * maxLength / len;
		y = y * maxLength / len;
	}
}

void Vec2::operator=( const Vec3& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
}

//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( float uniformScale ) const
{
	return Vec2(x * uniformScale, y * uniformScale);
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( const Vec2& vecToMultiply ) const
{
	return Vec2(x * vecToMultiply.x, y * vecToMultiply.y);
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator/( float inverseScale ) const
{
	return Vec2(x / inverseScale, y / inverseScale);
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator+=( const Vec2& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator-=( const Vec2& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator*=( const float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator/=( const float uniformDivisor )
{
	//GUARANTEE_OR_DIE( uniformDivisor != 0.f, "Cannot divided by 0!" );
	x /= uniformDivisor;
	y /= uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator=( const Vec2& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
}


//-----------------------------------------------------------------------------------------------
const Vec2 operator*( float uniformScale, const Vec2& vecToScale )
{
	return Vec2(vecToScale.x * uniformScale, vecToScale.y * uniformScale);
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator==( const Vec2& compare ) const
{
	if (x == compare.x && y == compare.y) {
		return true;
	}
	return false;
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator!=( const Vec2& compare ) const
{
	if (x == compare.x && y == compare.y) {
		return false;
	}
	return true;
}


