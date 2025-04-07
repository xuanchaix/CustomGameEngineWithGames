#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <memory.h>

Mat44::Mat44()
{
	m_values[Ix] = 1.f;			m_values[Jx] = 0.f;			m_values[Kx] = 0.f;  m_values[Tx] = 0.f;
	m_values[Iy] = 0.f;			m_values[Jy] = 1.f;			m_values[Ky] = 0.f;	 m_values[Ty] = 0.f;
	m_values[Iz] = 0.f;			m_values[Jz] = 0.f;			m_values[Kz] = 1.f;  m_values[Tz] = 0.f;
	m_values[Iw] = 0.f;			m_values[Jw] = 0.f;			m_values[Kw] = 0.f;	 m_values[Tw] = 1.f;
}

Mat44::Mat44( Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translation2D )
{
	m_values[Ix] = iBasis2D.x;	m_values[Jx] = jBasis2D.x;  m_values[Kx] = 0.f;  m_values[Tx] = translation2D.x;
	m_values[Iy] = iBasis2D.y;	m_values[Jy] = jBasis2D.y;  m_values[Ky] = 0.f;	 m_values[Ty] = translation2D.y;
	m_values[Iz] = 0.f;			m_values[Jz] = 0.f;			m_values[Kz] = 1.f;  m_values[Tz] = 0.f;
	m_values[Iw] = 0.f;			m_values[Jw] = 0.f;			m_values[Kw] = 0.f;	 m_values[Tw] = 1.f;
}

Mat44::Mat44( Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translation3D )
{
	m_values[Ix] = iBasis3D.x;	m_values[Jx] = jBasis3D.x;	m_values[Kx] = kBasis3D.x;	m_values[Tx] = translation3D.x;
	m_values[Iy] = iBasis3D.y;	m_values[Jy] = jBasis3D.y;	m_values[Ky] = kBasis3D.y;	m_values[Ty] = translation3D.y;
	m_values[Iz] = iBasis3D.z;	m_values[Jz] = jBasis3D.z;	m_values[Kz] = kBasis3D.z;	m_values[Tz] = translation3D.z;
	m_values[Iw] = 0.f;			m_values[Jw] = 0.f;			m_values[Kw] = 0.f;			m_values[Tw] = 1.f;
}

Mat44::Mat44( Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D )
{
	m_values[Ix] = iBasis4D.x;	m_values[Jx] = jBasis4D.x;	m_values[Kx] = kBasis4D.x;	m_values[Tx] = translation4D.x;
	m_values[Iy] = iBasis4D.y;	m_values[Jy] = jBasis4D.y;	m_values[Ky] = kBasis4D.y;	m_values[Ty] = translation4D.y;
	m_values[Iz] = iBasis4D.z;	m_values[Jz] = jBasis4D.z;	m_values[Kz] = kBasis4D.z;	m_values[Tz] = translation4D.z;
	m_values[Iw] = iBasis4D.w;	m_values[Jw] = jBasis4D.w;	m_values[Kw] = kBasis4D.w;	m_values[Tw] = translation4D.w;
}

Mat44::Mat44( float const* sixteenValueBasisMajor )
{
	memcpy( m_values, sixteenValueBasisMajor, 16 * sizeof(float) );
}

Mat44 const Mat44::CreateTranslation2D( Vec2 const& translationXY )
{
	return Mat44( Vec2( 1, 0 ), Vec2( 0, 1 ), translationXY );
}

Mat44 const Mat44::CreateTranslation3D( Vec3 const& translationXYZ )
{
	return Mat44( Vec3( 1, 0, 0 ), Vec3( 0, 1, 0 ), Vec3( 0, 0, 1 ), translationXYZ );
}

Mat44 const Mat44::CreateUniformScale2D( float uniformScaleXY )
{
	return Mat44( Vec2( uniformScaleXY, 0 ), Vec2( 0, uniformScaleXY ), Vec2( 0, 0 ) );
}

Mat44 const Mat44::CreateUniformScale3D( float uniformScaleXYZ )
{
	return Mat44( Vec3( uniformScaleXYZ, 0, 0 ), Vec3( 0, uniformScaleXYZ, 0 ), Vec3( 0, 0, uniformScaleXYZ ), Vec3( 0, 0, 0 ) );

}

Mat44 const Mat44::CreateNonUniformScale2D( Vec2 const& nonUniformScaleXY )
{
	return Mat44( Vec2( nonUniformScaleXY.x, 0 ), Vec2( 0, nonUniformScaleXY.y ), Vec2( 0, 0 ) );
}

Mat44 const Mat44::CreateNonUniformScale3D( Vec3 const& nonUniformScaleXYZ )
{
	return Mat44( Vec3( nonUniformScaleXYZ.x, 0, 0 ), Vec3( 0, nonUniformScaleXYZ.y, 0 ), Vec3( 0, 0, nonUniformScaleXYZ.z ), Vec3( 0, 0, 0 ) );
}

Mat44 const Mat44::CreateZRotationDegrees( float rotationDegreesAboutZ )
{
	float cosTheta = CosDegrees( rotationDegreesAboutZ );
	float sinTheta = SinDegrees( rotationDegreesAboutZ );
	float initArray[16];
	initArray[Ix] = cosTheta;	initArray[Jx] = -sinTheta;	initArray[Kx] = 0.f;	initArray[Tx] = 0.f;
	initArray[Iy] = sinTheta;	initArray[Jy] = cosTheta;	initArray[Ky] = 0.f;	initArray[Ty] = 0.f;
	initArray[Iz] = 0.f;		initArray[Jz] = 0.f;		initArray[Kz] = 1.f;	initArray[Tz] = 0.f;
	initArray[Iw] = 0.f;		initArray[Jw] = 0.f;		initArray[Kw] = 0.f;	initArray[Tw] = 1.f;
	return Mat44( initArray );
}

Mat44 const Mat44::CreateYRotationDegrees( float rotationDegreesAboutY )
{
	float cosTheta = CosDegrees( rotationDegreesAboutY );
	float sinTheta = SinDegrees( rotationDegreesAboutY );
	float initArray[16];
	initArray[Ix] = cosTheta;	initArray[Jx] = 0.f;	initArray[Kx] = sinTheta;	initArray[Tx] = 0.f;
	initArray[Iy] = 0.f;		initArray[Jy] = 1.f;	initArray[Ky] = 0.f;		initArray[Ty] = 0.f;
	initArray[Iz] =	-sinTheta;	initArray[Jz] = 0.f;	initArray[Kz] = cosTheta;	initArray[Tz] = 0.f;
	initArray[Iw] = 0.f;		initArray[Jw] = 0.f;	initArray[Kw] = 0.f;		initArray[Tw] = 1.f;
	return Mat44(initArray);
}

Mat44 const Mat44::CreateXRotationDegrees( float rotationDegreesAboutX )
{
	float cosTheta = CosDegrees( rotationDegreesAboutX );
	float sinTheta = SinDegrees( rotationDegreesAboutX );
	float initArray[16];
	initArray[Ix] = 1.f;	initArray[Jx] = 0.f;		initArray[Kx] = 0.f;		initArray[Tx] = 0.f;
	initArray[Iy] = 0.f;	initArray[Jy] = cosTheta;	initArray[Ky] = -sinTheta;	initArray[Ty] = 0.f;
	initArray[Iz] = 0.f;	initArray[Jz] = sinTheta;	initArray[Kz] = cosTheta;	initArray[Tz] = 0.f;
	initArray[Iw] = 0.f;	initArray[Jw] = 0.f;		initArray[Kw] = 0.f;		initArray[Tw] = 1.f;
	return Mat44(initArray);
}

Vec2 const Mat44::TransformVectorQuantity2D( Vec2 const& vectorQuantityXY ) const
{
	float retX = m_values[Ix] * vectorQuantityXY.x + m_values[Jx] * vectorQuantityXY.y;
	float retY = m_values[Iy] * vectorQuantityXY.x + m_values[Jy] * vectorQuantityXY.y;
	return Vec2( retX, retY );
}

Vec3 const Mat44::TransformVectorQuantity3D( Vec3 const& vectorQuantityXYZ ) const
{
	float retX = m_values[Ix] * vectorQuantityXYZ.x + m_values[Jx] * vectorQuantityXYZ.y + m_values[Kx] * vectorQuantityXYZ.z;
	float retY = m_values[Iy] * vectorQuantityXYZ.x + m_values[Jy] * vectorQuantityXYZ.y + m_values[Ky] * vectorQuantityXYZ.z;
	float retZ = m_values[Iz] * vectorQuantityXYZ.x + m_values[Jz] * vectorQuantityXYZ.y + m_values[Kz] * vectorQuantityXYZ.z;
	return Vec3( retX, retY, retZ );
}

Vec2 const Mat44::TransformPosition2D( Vec2 const& positionXY ) const
{
	float retX = m_values[Ix] * positionXY.x + m_values[Jx] * positionXY.y + m_values[Tx];
	float retY = m_values[Iy] * positionXY.x + m_values[Jy] * positionXY.y + m_values[Ty];
	return Vec2( retX, retY );
}

Vec3 const Mat44::TransformPosition3D( Vec3 const& position3D ) const
{
	float retX = m_values[Ix] * position3D.x + m_values[Jx] * position3D.y + m_values[Kx] * position3D.z + m_values[Tx];
	float retY = m_values[Iy] * position3D.x + m_values[Jy] * position3D.y + m_values[Ky] * position3D.z + m_values[Ty];
	float retZ = m_values[Iz] * position3D.x + m_values[Jz] * position3D.y + m_values[Kz] * position3D.z + m_values[Tz];
	return Vec3( retX, retY, retZ );
}

Vec4 const Mat44::TransformHomogeneous3D( Vec4 const& homogeneousPoint3D ) const
{
	float retX = m_values[Ix] * homogeneousPoint3D.x + m_values[Jx] * homogeneousPoint3D.y + m_values[Kx] * homogeneousPoint3D.z + m_values[Tx] * homogeneousPoint3D.w;
	float retY = m_values[Iy] * homogeneousPoint3D.x + m_values[Jy] * homogeneousPoint3D.y + m_values[Ky] * homogeneousPoint3D.z + m_values[Ty] * homogeneousPoint3D.w;
	float retZ = m_values[Iz] * homogeneousPoint3D.x + m_values[Jz] * homogeneousPoint3D.y + m_values[Kz] * homogeneousPoint3D.z + m_values[Tz] * homogeneousPoint3D.w;
	float retW = m_values[Iw] * homogeneousPoint3D.x + m_values[Jw] * homogeneousPoint3D.y + m_values[Kw] * homogeneousPoint3D.z + m_values[Tw] * homogeneousPoint3D.w;
	return Vec4( retX, retY, retZ, retW );
}

float* Mat44::GetAsFloatArray()
{
	return m_values;
}

float const* Mat44::GetAsFloatArray() const
{
	return m_values;
}

Vec2 const Mat44::GetIBasis2D() const
{
	return Vec2( m_values[Ix], m_values[Iy] );
}

Vec2 const Mat44::GetJBasis2D() const
{
	return Vec2( m_values[Jx], m_values[Jy] );
}

Vec2 const Mat44::GetTranslation2D() const
{
	return Vec2( m_values[Tx], m_values[Ty] );
}

Vec3 const Mat44::GetIBasis3D() const
{
	return Vec3( m_values[Ix], m_values[Iy], m_values[Iz] );
}

Vec3 const Mat44::GetJBasis3D() const
{
	return Vec3( m_values[Jx], m_values[Jy], m_values[Jz] );
}

Vec3 const Mat44::GetKBasis3D() const
{
	return Vec3( m_values[Kx], m_values[Ky], m_values[Kz] );
}

Vec3 const Mat44::GetTranslation3D() const
{
	return Vec3( m_values[Tx], m_values[Ty], m_values[Tz] );
}

Vec4 const Mat44::GetIBasis4D() const
{
	return Vec4( m_values[Ix], m_values[Iy], m_values[Iz], m_values[Iw] );
}

Vec4 const Mat44::GetJBasis4D() const
{
	return Vec4( m_values[Jx], m_values[Jy], m_values[Jz], m_values[Jw] );
}

Vec4 const Mat44::GetKBasis4D() const
{
	return Vec4( m_values[Kx], m_values[Ky], m_values[Kz], m_values[Kw] );
}

Vec4 const Mat44::GetTranslation4D() const
{
	return Vec4( m_values[Tx], m_values[Ty], m_values[Tz], m_values[Tw] );
}

void Mat44::SetTranslation2D( Vec2 const& translationXY )
{
	m_values[Tx] = translationXY.x;
	m_values[Ty] = translationXY.y;
	m_values[Tz] = 0.f;
	m_values[Tw] = 1.f;
}

void Mat44::SetTranslation3D( Vec3 const& translationXYZ )
{
	m_values[Tx] = translationXYZ.x;
	m_values[Ty] = translationXYZ.y;
	m_values[Tz] = translationXYZ.z;
	m_values[Tw] = 1.f;
}

void Mat44::SetIJ2D( Vec2 const& iBasis2D, Vec2 const& jBasis2D )
{
	m_values[Ix] = iBasis2D.x;	m_values[Jx] = jBasis2D.x;
	m_values[Iy] = iBasis2D.y;	m_values[Jy] = jBasis2D.y;
	m_values[Iz] = 0.f;			m_values[Jz] = 0.f;
	m_values[Iw] = 0.f; 		m_values[Jw] = 0.f;
}

void Mat44::SetIJT2D( Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translationXY )
{
	m_values[Ix] = iBasis2D.x;  m_values[Jx] = jBasis2D.x;  m_values[Tx] = translationXY.x;
	m_values[Iy] = iBasis2D.y;  m_values[Jy] = jBasis2D.y;  m_values[Ty] = translationXY.y;
	m_values[Iz] = 0.f;			m_values[Jz] = 0.f;			m_values[Tz] = 0.f;
	m_values[Iw] = 0.f;			m_values[Jw] = 0.f;			m_values[Tw] = 1.f;
}

void Mat44::SetIJK3D( Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D )
{
	m_values[Ix] = iBasis3D.x;  m_values[Jx] = jBasis3D.x;  m_values[Kx] = kBasis3D.x;
	m_values[Iy] = iBasis3D.y;  m_values[Jy] = jBasis3D.y;  m_values[Ky] = kBasis3D.y;
	m_values[Iz] = iBasis3D.z;  m_values[Jz] = jBasis3D.z;  m_values[Kz] = kBasis3D.z;
	m_values[Iw] = 0.f;			m_values[Jw] = 0.f;			m_values[Kw] = 0.f;
}

void Mat44::SetIJKT3D( Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translationXYZ )
{
	m_values[Ix] = iBasis3D.x;  m_values[Jx] = jBasis3D.x;  m_values[Kx] = kBasis3D.x;  m_values[Tx] = translationXYZ.x;
	m_values[Iy] = iBasis3D.y;  m_values[Jy] = jBasis3D.y;  m_values[Ky] = kBasis3D.y;  m_values[Ty] = translationXYZ.y;
	m_values[Iz] = iBasis3D.z;  m_values[Jz] = jBasis3D.z;  m_values[Kz] = kBasis3D.z;  m_values[Tz] = translationXYZ.z;
	m_values[Iw] = 0.f;			m_values[Jw] = 0.f;			m_values[Kw] = 0.f;			m_values[Tw] = 1.f;
}

void Mat44::SetIJKT4D( Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D )
{
	m_values[Ix] = iBasis4D.x;	m_values[Jx] = jBasis4D.x;	m_values[Kx] = kBasis4D.x;	m_values[Tx] = translation4D.x;
	m_values[Iy] = iBasis4D.y;	m_values[Jy] = jBasis4D.y;	m_values[Ky] = kBasis4D.y;	m_values[Ty] = translation4D.y;
	m_values[Iz] = iBasis4D.z;	m_values[Jz] = jBasis4D.z;	m_values[Kz] = kBasis4D.z;	m_values[Tz] = translation4D.z;
	m_values[Iw] = iBasis4D.w;	m_values[Jw] = jBasis4D.w;	m_values[Kw] = kBasis4D.w;	m_values[Tw] = translation4D.w;
}

//-----------------------------------------
// this function makes append matrix insert into the most front of matrix multiply sequence 
// ( in other words: do to the point first )
// in column major:
// thisMatrix = thisMatrix * appendMat;
// [this] = [this][append]
void Mat44::Append( Mat44 const& appendMat )
{
	float tempValues[16];
	memcpy( tempValues, m_values, 16 * sizeof( float ) );
	m_values[Ix] = tempValues[Ix] * appendMat.m_values[Ix] + tempValues[Jx] * appendMat.m_values[Iy] + tempValues[Kx] * appendMat.m_values[Iz] + tempValues[Tx] * appendMat.m_values[Iw];
	m_values[Jx] = tempValues[Ix] * appendMat.m_values[Jx] + tempValues[Jx] * appendMat.m_values[Jy] + tempValues[Kx] * appendMat.m_values[Jz] + tempValues[Tx] * appendMat.m_values[Jw];
	m_values[Kx] = tempValues[Ix] * appendMat.m_values[Kx] + tempValues[Jx] * appendMat.m_values[Ky] + tempValues[Kx] * appendMat.m_values[Kz] + tempValues[Tx] * appendMat.m_values[Kw];
	m_values[Tx] = tempValues[Ix] * appendMat.m_values[Tx] + tempValues[Jx] * appendMat.m_values[Ty] + tempValues[Kx] * appendMat.m_values[Tz] + tempValues[Tx] * appendMat.m_values[Tw];
	m_values[Iy] = tempValues[Iy] * appendMat.m_values[Ix] + tempValues[Jy] * appendMat.m_values[Iy] + tempValues[Ky] * appendMat.m_values[Iz] + tempValues[Ty] * appendMat.m_values[Iw];
	m_values[Jy] = tempValues[Iy] * appendMat.m_values[Jx] + tempValues[Jy] * appendMat.m_values[Jy] + tempValues[Ky] * appendMat.m_values[Jz] + tempValues[Ty] * appendMat.m_values[Jw];
	m_values[Ky] = tempValues[Iy] * appendMat.m_values[Kx] + tempValues[Jy] * appendMat.m_values[Ky] + tempValues[Ky] * appendMat.m_values[Kz] + tempValues[Ty] * appendMat.m_values[Kw];
	m_values[Ty] = tempValues[Iy] * appendMat.m_values[Tx] + tempValues[Jy] * appendMat.m_values[Ty] + tempValues[Ky] * appendMat.m_values[Tz] + tempValues[Ty] * appendMat.m_values[Tw];
	m_values[Iz] = tempValues[Iz] * appendMat.m_values[Ix] + tempValues[Jz] * appendMat.m_values[Iy] + tempValues[Kz] * appendMat.m_values[Iz] + tempValues[Tz] * appendMat.m_values[Iw];
	m_values[Jz] = tempValues[Iz] * appendMat.m_values[Jx] + tempValues[Jz] * appendMat.m_values[Jy] + tempValues[Kz] * appendMat.m_values[Jz] + tempValues[Tz] * appendMat.m_values[Jw];
	m_values[Kz] = tempValues[Iz] * appendMat.m_values[Kx] + tempValues[Jz] * appendMat.m_values[Ky] + tempValues[Kz] * appendMat.m_values[Kz] + tempValues[Tz] * appendMat.m_values[Kw];
	m_values[Tz] = tempValues[Iz] * appendMat.m_values[Tx] + tempValues[Jz] * appendMat.m_values[Ty] + tempValues[Kz] * appendMat.m_values[Tz] + tempValues[Tz] * appendMat.m_values[Tw];
	m_values[Iw] = tempValues[Iw] * appendMat.m_values[Ix] + tempValues[Jw] * appendMat.m_values[Iy] + tempValues[Kw] * appendMat.m_values[Iz] + tempValues[Tw] * appendMat.m_values[Iw];
	m_values[Jw] = tempValues[Iw] * appendMat.m_values[Jx] + tempValues[Jw] * appendMat.m_values[Jy] + tempValues[Kw] * appendMat.m_values[Jz] + tempValues[Tw] * appendMat.m_values[Jw];
	m_values[Kw] = tempValues[Iw] * appendMat.m_values[Kx] + tempValues[Jw] * appendMat.m_values[Ky] + tempValues[Kw] * appendMat.m_values[Kz] + tempValues[Tw] * appendMat.m_values[Kw];
	m_values[Tw] = tempValues[Iw] * appendMat.m_values[Tx] + tempValues[Jw] * appendMat.m_values[Ty] + tempValues[Kw] * appendMat.m_values[Tz] + tempValues[Tw] * appendMat.m_values[Tw];

	/*
	#include "Engine/Core/ErrorWarningAssert.hpp"
	DebuggerPrintf("%.2f %.2f %.2f %.2f\n%.2f %.2f %.2f %.2f\n%.2f %.2f %.2f %.2f\n%.2f %.2f %.2f %.2f\n",
		m_values[Ix], m_values[Jx], m_values[Kx], m_values[Tx],
		m_values[Iy], m_values[Jy], m_values[Ky], m_values[Ty],
		m_values[Iz], m_values[Jz], m_values[Kz], m_values[Tz],
		m_values[Iw], m_values[Jw], m_values[Kw], m_values[Tw]
	);*/
}

void Mat44::AppendZRotation( float degreesRotationAboutZ )
{
	Append( Mat44::CreateZRotationDegrees( degreesRotationAboutZ ) );
}

void Mat44::AppendYRotation( float degreesRotationAboutY )
{
	Append( Mat44::CreateYRotationDegrees( degreesRotationAboutY ) );
}

void Mat44::AppendXRotation( float degreesRotationAboutX )
{
	Append( Mat44::CreateXRotationDegrees( degreesRotationAboutX ) );
}

void Mat44::AppendTranslation2D( Vec2 const& translationXY )
{
	Append( Mat44::CreateTranslation2D( translationXY ) );
}

void Mat44::AppendTranslation3D( Vec3 const& translationXYZ )
{
	Append( Mat44::CreateTranslation3D( translationXYZ ) );
}

void Mat44::AppendScaleUniform2D( float uniformScaleXY )
{
	Append( Mat44::CreateUniformScale2D( uniformScaleXY ) );
}

void Mat44::AppendScaleUniform3D( float uniformScaleXYZ )
{
	Append( Mat44::CreateUniformScale3D( uniformScaleXYZ ) );
}

void Mat44::AppendScaleNonUniform2D( Vec2 const& nonUniformScaleXY )
{
	Append( Mat44::CreateNonUniformScale2D( nonUniformScaleXY ) );
}

void Mat44::AppendScaleNonUniform3D( Vec3 const& nonUniformScaleXYZ )
{
	Append( Mat44::CreateNonUniformScale3D( nonUniformScaleXYZ ) );
}

Mat44 const Mat44::CreateOrthoProjection( float left, float right, float bottom, float top, float near, float far )
{
	Mat44 retMat;
	retMat.m_values[Mat44::Ix] = 2.f / (right - left);
	retMat.m_values[Mat44::Jy] = 2.f / (top - bottom);
	retMat.m_values[Mat44::Kz] = -1.f / (near - far);
	retMat.m_values[Mat44::Tx] = (left + right) / (left - right);
	retMat.m_values[Mat44::Ty] = (top + bottom) / (bottom - top);
	retMat.m_values[Mat44::Tz] =  near / (near - far);
	return retMat;
}

Mat44 const Mat44::CreatePerspectiveProjection( float fov, float aspect, float near, float far )
{
	Mat44 retMat;
	float s = CosDegrees( fov * 0.5f ) / SinDegrees( fov * 0.5f );
	retMat.m_values[Mat44::Ix] = s / aspect;
	retMat.m_values[Mat44::Jy] = s;
	retMat.m_values[Mat44::Kz] = far / (far - near);
	retMat.m_values[Mat44::Kw] = 1.f;
	retMat.m_values[Mat44::Tz] = -far * near / (far - near);
	retMat.m_values[Mat44::Tw] = 0.f;
	return retMat;
}

void Mat44::Transpose()
{
	Mat44 originMat( *this );
	m_values[Ix] = originMat.m_values[Ix];
	m_values[Iy] = originMat.m_values[Jx];
	m_values[Iz] = originMat.m_values[Kx];
	m_values[Iw] = originMat.m_values[Tx];

	m_values[Jx] = originMat.m_values[Iy];
	m_values[Jy] = originMat.m_values[Jy];
	m_values[Jz] = originMat.m_values[Ky];
	m_values[Jw] = originMat.m_values[Ty];

	m_values[Kx] = originMat.m_values[Iz];
	m_values[Ky] = originMat.m_values[Jz];
	m_values[Kz] = originMat.m_values[Kz];
	m_values[Kw] = originMat.m_values[Tz];

	m_values[Tx] = originMat.m_values[Iw];
	m_values[Ty] = originMat.m_values[Jw];
	m_values[Tz] = originMat.m_values[Kw];
	m_values[Tw] = originMat.m_values[Tw];
}

Mat44 const Mat44::GetOrthonormalInverse() const
{
	Mat44 retMat;
	retMat.m_values[Ix] = m_values[Ix];
	retMat.m_values[Iy] = m_values[Jx];
	retMat.m_values[Iz] = m_values[Kx];

	retMat.m_values[Jx] = m_values[Iy];
	retMat.m_values[Jy] = m_values[Jy];
	retMat.m_values[Jz] = m_values[Ky];

	retMat.m_values[Kx] = m_values[Iz];
	retMat.m_values[Ky] = m_values[Jz];
	retMat.m_values[Kz] = m_values[Kz];

	retMat.m_values[Tx] = -(m_values[Ix] * m_values[Tx] + m_values[Iy] * m_values[Ty] + m_values[Iz] * m_values[Tz]);
	retMat.m_values[Ty] = -(m_values[Jx] * m_values[Tx] + m_values[Jy] * m_values[Ty] + m_values[Jz] * m_values[Tz]);
	retMat.m_values[Tz] = -(m_values[Kx] * m_values[Tx] + m_values[Ky] * m_values[Ty] + m_values[Kz] * m_values[Tz]);
	
	return retMat;
}

void Mat44::Orthonormalize_IFwd_JLeft_KUp()
{
	Vec3 iBasis = GetIBasis3D();
	iBasis.Normalize();
	
	Vec3 kBasis = GetKBasis3D();
	kBasis -= DotProduct3D( kBasis, iBasis ) * iBasis;
	kBasis.Normalize();

	Vec3 jBasis = GetJBasis3D();
	jBasis -= DotProduct3D( iBasis, jBasis ) * iBasis;
	jBasis -= DotProduct3D( kBasis, jBasis ) * kBasis;
	jBasis.Normalize();

	SetIJK3D( iBasis, jBasis, kBasis );
}
