#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/Plane.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/ConvexHull2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include <Math.h>
#include <cmath>


float Absf( float value )
{
	return value <= -0.f ? -value : value;
}

float Minf( float a, float b )
{
	return a < b ? a : b;
}

float Maxf( float a, float b )
{
	return a > b ? a : b;
}

float NormalizeByte( unsigned char byte )
{
	return (float)byte / 255.f;
}

unsigned char DenormalizeByte( float floatNum )
{
	//return (unsigned char)(floatNum * 255.f);
	return (unsigned char)GetClamped( floatNum * 256.f, 0.f, 255.f );
}

bool DoFloatRangeOverlap( FloatRange const& f1, FloatRange const& f2 )
{
	return f1.m_max > f2.m_min && f2.m_max > f1.m_min;
}

float GetClamped( float value, float minValue, float maxValue )
{
	if (value <= minValue) {
		return minValue;
	}
	if (value >= maxValue) {
		return maxValue;
	}
	return value;
}

int GetClamped( int value, int minValue, int maxValue )
{
	if (value <= minValue) {
		return minValue;
	}
	if (value >= maxValue) {
		return maxValue;
	}
	return value;
}

float GetClampedZeroToOne( float value )
{
	if (value < 0.f) {
		return 0.f;
	}
	if (value > 1.f) {
		return 1.f;
	}
	return value;
}

float Interpolate( float start, float end, float fractionTowardEnd )
{
	return start + (end - start) * fractionTowardEnd;
}

Vec2 Interpolate( Vec2 const& start, Vec2 const& end, float fractionTowardEnd )
{
	return Vec2( Interpolate( start.x, end.x, fractionTowardEnd ), Interpolate( start.y, end.y, fractionTowardEnd ) );
}

Vec4 Interpolate( Vec4 const& start, Vec4 const& end, float fractionTowardEnd )
{
	return Vec4( Interpolate( start.x, end.x, fractionTowardEnd ), Interpolate( start.y, end.y, fractionTowardEnd ), Interpolate( start.z, end.z, fractionTowardEnd ), Interpolate( start.w, end.w, fractionTowardEnd ) );
}

Vec3 Interpolate( Vec3 const& start, Vec3 const& end, float fractionTowardEnd )
{
	return Vec3( Interpolate( start.x, end.x, fractionTowardEnd ), Interpolate( start.y, end.y, fractionTowardEnd ), Interpolate( start.z, end.z, fractionTowardEnd ) );
}

float GetFractionWithinRange( float value, float rangeStart, float rangeEnd )
{
	//GUARANTEE_OR_DIE( rangeStart == rangeEnd, "NEED: rangeStart shouldn't equal to rangeEnd" );
	if (rangeStart == rangeEnd) {
		return 0.5f;
	}
	return (value - rangeStart) / (rangeEnd - rangeStart);
}

float RangeMap( float inValue, float inStart, float inEnd, float outStart, float outEnd )
{
	return Interpolate( outStart, outEnd, GetFractionWithinRange( inValue, inStart, inEnd ) );
}

float RangeMapClamped( float inValue, float inStart, float inEnd, float outStart, float outEnd )
{
	if (inValue < inStart) {
		inValue = inStart;
	}
	if (inValue > inEnd) {
		inValue = inEnd;
	}
	return Interpolate( outStart, outEnd, GetFractionWithinRange( inValue, inStart, inEnd ) );
}

int RoundDownToInt( float value )
{
	return (int)floorf( value );
}

int RoundToInt( float value )
{
	return (int)floorf( value + 0.5f );
}

float ConvertDegreesToRadians( float degrees )
{
	return degrees * PI / 180.f;
}

float ConvertRadiansToDegrees( float radians )
{
	return radians / PI * 180.f;
}

float CosDegrees( float degrees )
{
	return cosf( ConvertDegreesToRadians( degrees ) );
}

float SinDegrees( float degrees )
{
	return sinf( ConvertDegreesToRadians( degrees ) );
}

float CosRadians( float radians )
{
	return cosf( radians );
}

float SinRadians( float radians )
{
	return sinf( radians );
}

float Atan2Degrees( float y, float x )
{
	return ConvertRadiansToDegrees( atan2f( y, x ) );
}

float Atan2Radians( float y, float x )
{
	return atan2f( y, x );
}

float GetShortestAngularDispDegrees( float startDegrees, float endDegrees )
{
	float targetDegrees = endDegrees - startDegrees;
	while (targetDegrees > 180.f) {
		targetDegrees -= 360.f;
	}
	while (targetDegrees < -180.f) {
		targetDegrees += 360.f;
	}
	return targetDegrees;
}

float GetTurnedTowardDegrees( float currentDegrees, float goalDegrees, float maxDeltaDegrees )
{
	GUARANTEE_OR_DIE( maxDeltaDegrees >= 0.f, "NEED: delta degrees should be larger than 0" );
	if (maxDeltaDegrees >= 180.f) {
		return goalDegrees;
	}
	while (currentDegrees > 360.f) {
		currentDegrees -= 360.f;
	}
	while (currentDegrees < 0.f) {
		currentDegrees += 360.f;
	}
	while (goalDegrees > 360.f) {
		goalDegrees -= 360.f;
	}
	while (goalDegrees < 0.f) {
		goalDegrees += 360.f;
	}
	float minDegrees = currentDegrees - maxDeltaDegrees;
	float maxDegrees = currentDegrees + maxDeltaDegrees;
	if (goalDegrees < maxDegrees && goalDegrees > minDegrees) {
		return goalDegrees;
	}
	else if (Absf( GetShortestAngularDispDegrees( minDegrees, goalDegrees ) ) > Absf( GetShortestAngularDispDegrees( maxDegrees, goalDegrees ))) {
		while (maxDegrees > 360.f) {
			maxDegrees -= 360.f;
		}
		while (maxDegrees < 0.f) {
			maxDegrees += 360.f;
		}
		return maxDegrees;
	}
	else {
		while (minDegrees > 360.f) {
			minDegrees -= 360.f;
		}
		while (minDegrees < 0.f) {
			minDegrees += 360.f;
		}
		return minDegrees;
	}
}

float GetAngleDegreesBetweenVectors2D( Vec2 const& a, Vec2 const& b )
{
	return ConvertRadiansToDegrees( acosf( DotProduct2D( a, b ) / a.GetLength() / b.GetLength() ) );
}

Vec2 GetInversedOrthonormaliBasis( Vec2 const& iBasisNormal )
{
	return Vec2( iBasisNormal.x, -iBasisNormal.y );
}

float NormalizeDegrees180( float degrees )
{
	while (degrees > 180.f) {
		degrees -= 360.f;
	}
	while (degrees <= -180.f) {
		degrees += 360.f;
	}
	return degrees;
}

float DotProduct2D( Vec2 const& a, Vec2 const& b )
{
	return a.x * b.x + a.y * b.y;
}

float DotProduct3D( Vec3 const& a, Vec3 const& b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

float DotProduct4D( Vec4 const& a, Vec4 const& b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

float CrossProduct2D( Vec2 const& a, Vec2 const& b )
{
	return a.x * b.y - b.x * a.y;
}

Vec3 CrossProduct3D( Vec3 const& a, Vec3 const& b )
{
	return Vec3( a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x );
}

float GetDistance2D( Vec2 const& positionA, Vec2 const& positionB )
{
	return sqrtf( (positionA.x - positionB.x) * (positionA.x - positionB.x) + (positionA.y - positionB.y) * (positionA.y - positionB.y) );
}

float GetDistanceSquared2D( Vec2 const& positionA, Vec2 const& positionB )
{
	return (positionA.x - positionB.x) * (positionA.x - positionB.x) + (positionA.y - positionB.y) * (positionA.y - positionB.y);
}

float GetDistance3D( Vec3 const& positionA, Vec3 const& positionB )
{
	return sqrtf( (positionA.x - positionB.x) * (positionA.x - positionB.x) 
				+ (positionA.y - positionB.y) * (positionA.y - positionB.y) 
				+ (positionA.z - positionB.z) * (positionA.z - positionB.z) 
				);
}

float GetDistanceSquared3D( Vec3 const& positionA, Vec3 const& positionB )
{
	return (positionA.x - positionB.x) * (positionA.x - positionB.x)
		 + (positionA.y - positionB.y) * (positionA.y - positionB.y)
		 + (positionA.z - positionB.z) * (positionA.z - positionB.z);
}

float GetDistanceXY3D( Vec3 const& positionA, Vec3 const& positionB )
{
	return sqrtf( (positionA.x - positionB.x) * (positionA.x - positionB.x) + (positionA.y - positionB.y) * (positionA.y - positionB.y) );
}

float GetDistanceXYSquared3D( Vec3 const& positionA, Vec3 const& positionB )
{
	return (positionA.x - positionB.x) * (positionA.x - positionB.x) + (positionA.y - positionB.y) * (positionA.y - positionB.y);
}

int GetTaxicabDistance2D( IntVec2 const& pointA, IntVec2 const& pointB )
{
	return abs( pointA.x - pointB.x ) + abs( pointA.y - pointB.y );
}

float GetTaxicabDistance2D( Vec2 const& positionA, Vec2 const& positionB )
{
	return abs( positionA.x - positionB.x ) + abs( positionA.y - positionB.y );
}

float GetProjectedLength2D( Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto )
{
	return DotProduct2D( vectorToProjectOnto.GetNormalized(), vectorToProject );
}

Vec2 GetProjectedOnto2D( Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto )
{
	Vec2 normalizedVectorToProjectOnto = vectorToProjectOnto.GetNormalized();
	return normalizedVectorToProjectOnto * DotProduct2D( normalizedVectorToProjectOnto, vectorToProject );
}

Vec2 GetRandomPointOnUnitCircle2D()
{
	Vec2 point;
	float length;
	do {
		point.x = g_engineRNG->RollRandomFloatZeroToOne() * 2.f - 1.f;
		point.y = g_engineRNG->RollRandomFloatZeroToOne() * 2.f - 1.f;
		length = point.GetLengthSquared();
	} while (length > 1.f || length < 0.0001f);
	return point / sqrtf( length );
}

Vec2 GetRandomPointInUnitDisc2D()
{
	Vec2 point;
	float length;
	do {
		point.x = g_engineRNG->RollRandomFloatZeroToOne() * 2.f - 1.f;
		point.y = g_engineRNG->RollRandomFloatZeroToOne() * 2.f - 1.f;
		length = point.GetLengthSquared();
	} while (length > 1.f);
	return point;
}

Vec2 GetRandomPointInDisc2D( float discRadius, Vec2 const& discCenter )
{
	return GetRandomPointInUnitDisc2D() * discRadius + discCenter;
}

Vec3 GetRandomDirection3D()
{
	Vec3 point;
	float length;
	do {
		point.x = g_engineRNG->RollRandomFloatZeroToOne() * 2.f - 1.f;
		point.y = g_engineRNG->RollRandomFloatZeroToOne() * 2.f - 1.f;
		point.z = g_engineRNG->RollRandomFloatZeroToOne() * 2.f - 1.f;
		length = point.GetLengthSquared();
	} while (length > 1.f || length < 0.0001f);
	return point / sqrtf( length );
}

Vec2 GetRandomPointInAABB2D( AABB2 const& box )
{
	float x = g_engineRNG->RollRandomFloatInRange( box.m_mins.x, box.m_maxs.x );
	float y = g_engineRNG->RollRandomFloatInRange( box.m_mins.y, box.m_maxs.y );
	return Vec2( x, y );
}

Vec3 GetRandomPointInAABB3D( AABB3 const& box )
{
	float x = g_engineRNG->RollRandomFloatInRange( box.m_mins.x, box.m_maxs.x );
	float y = g_engineRNG->RollRandomFloatInRange( box.m_mins.y, box.m_maxs.y );
	float z = g_engineRNG->RollRandomFloatInRange( box.m_mins.z, box.m_maxs.z );
	return Vec3( x, y, z );
}

Vec3 GetRandomDirectionInCone3D( Vec3 const& coneForward, float maxHalfYaw, float maxHalfPitch )
{
	EulerAngles orientation = coneForward.GetOrientationEulerAngles();
	float deltaYaw = g_engineRNG->RollRandomFloatInRange( -maxHalfYaw, maxHalfYaw );
	float deltaPitch = g_engineRNG->RollRandomFloatInRange( -maxHalfPitch, maxHalfPitch );
	orientation.m_yawDegrees += deltaYaw;
	orientation.m_pitchDegrees += deltaPitch;
	return orientation.GetIFwd();
}

bool DoDiscsOverlap( Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB )
{
	return GetDistanceSquared2D( centerA, centerB ) < (radiusA + radiusB) * (radiusA + radiusB);
}

bool DoSpheresOverlap( Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB )
{
	return GetDistanceSquared3D( centerA, centerB ) < (radiusA + radiusB) * (radiusA + radiusB);
}

bool DoZCylindersOverlap3D( Vec2 const& cylinder1CenterXY, float cylinder1Radius, float cylinder1minZ, float cylinder1maxZ, Vec2 const& cylinder2CenterXY, float cylinder2Radius, float cylinder2minZ, float cylinder2maxZ )
{
	if (DoDiscsOverlap( cylinder1CenterXY, cylinder1Radius, cylinder2CenterXY, cylinder2Radius ) 
		&& cylinder1maxZ > cylinder2minZ && cylinder2maxZ > cylinder1minZ) {
		return true;
	}
	return false;
}

bool DoPlanesOverlap3D( Plane3 const& plane1, Plane3 const& plane2 )
{
	Vec3 n = CrossProduct3D( plane1.m_normal, plane2.m_normal );
	float nLengthSquared = n.GetLengthSquared();
	if (abs( nLengthSquared ) < 0.0001f) {
		return false;
	}
	return true;
}

bool DoOBB3sOverlap3D( OBB3 const& obb1, OBB3 const& obb2 )
{
	// translate obb2 to obb1's local space
	Mat44 obb1Matrix( obb1.m_iBasis, obb1.m_jBasis, obb1.m_kBasis, obb1.m_center );
	Mat44 transformMatrix = obb1Matrix.GetOrthonormalInverse();

	Vec3 localCenter = transformMatrix.TransformPosition3D( obb2.m_center );
	Vec3 localiBasis = transformMatrix.TransformVectorQuantity3D( obb2.m_iBasis );
	Vec3 localjBasis = transformMatrix.TransformVectorQuantity3D( obb2.m_jBasis );
	Vec3 localkBasis = transformMatrix.TransformVectorQuantity3D( obb2.m_kBasis );
	Vec3 localHalfDim = transformMatrix.TransformVectorQuantity3D( obb2.m_halfDimensions );

	OBB3 localOBB3D( localCenter, localiBasis, localjBasis, localkBasis, localHalfDim );
	AABB3 localAABB3D( -obb1.m_halfDimensions, obb1.m_halfDimensions );

	return DoAABB3AndOBB3Overlap3D( localAABB3D, localOBB3D );
}

bool DoAABB3AndOBB3Overlap3D( AABB3 const& aabb, OBB3 const& obb )
{
	Vec3 p1 = obb.m_center + obb.m_iBasis * obb.m_halfDimensions.x + obb.m_jBasis * obb.m_halfDimensions.y + obb.m_kBasis * obb.m_halfDimensions.z;
	Vec3 p2 = obb.m_center + obb.m_iBasis * obb.m_halfDimensions.x + obb.m_jBasis * obb.m_halfDimensions.y - obb.m_kBasis * obb.m_halfDimensions.z;
	Vec3 p3 = obb.m_center + obb.m_iBasis * obb.m_halfDimensions.x - obb.m_jBasis * obb.m_halfDimensions.y + obb.m_kBasis * obb.m_halfDimensions.z;
	Vec3 p4 = obb.m_center + obb.m_iBasis * obb.m_halfDimensions.x - obb.m_jBasis * obb.m_halfDimensions.y - obb.m_kBasis * obb.m_halfDimensions.z;
	Vec3 p5 = obb.m_center - obb.m_iBasis * obb.m_halfDimensions.x + obb.m_jBasis * obb.m_halfDimensions.y + obb.m_kBasis * obb.m_halfDimensions.z;
	Vec3 p6 = obb.m_center - obb.m_iBasis * obb.m_halfDimensions.x + obb.m_jBasis * obb.m_halfDimensions.y - obb.m_kBasis * obb.m_halfDimensions.z;
	Vec3 p7 = obb.m_center - obb.m_iBasis * obb.m_halfDimensions.x - obb.m_jBasis * obb.m_halfDimensions.y + obb.m_kBasis * obb.m_halfDimensions.z;
	Vec3 p8 = obb.m_center - obb.m_iBasis * obb.m_halfDimensions.x - obb.m_jBasis * obb.m_halfDimensions.y - obb.m_kBasis * obb.m_halfDimensions.z;

	float minx = Minf( p1.x, Minf( p2.x, Minf( p3.x, Minf( p4.x, Minf( p5.x, Minf( p6.x, Minf( p7.x, p8.x ) ) ) ) ) ) );
	float maxx = Maxf( p1.x, Maxf( p2.x, Maxf( p3.x, Maxf( p4.x, Maxf( p5.x, Maxf( p6.x, Maxf( p7.x, p8.x ) ) ) ) ) ) );
	if (minx >= aabb.m_maxs.x || maxx <= aabb.m_mins.x) {
		return false;
	}

	float miny = Minf( p1.y, Minf( p2.y, Minf( p3.y, Minf( p4.y, Minf( p5.y, Minf( p6.y, Minf( p7.y, p8.y ) ) ) ) ) ) );
	float maxy = Maxf( p1.y, Maxf( p2.y, Maxf( p3.y, Maxf( p4.y, Maxf( p5.y, Maxf( p6.y, Maxf( p7.y, p8.y ) ) ) ) ) ) );
	if (miny >= aabb.m_maxs.y || maxy <= aabb.m_mins.y) {
		return false;
	}

	float minz = Minf( p1.z, Minf( p2.z, Minf( p3.z, Minf( p4.z, Minf( p5.z, Minf( p6.z, Minf( p7.z, p8.z ) ) ) ) ) ) );
	float maxz = Maxf( p1.z, Maxf( p2.z, Maxf( p3.z, Maxf( p4.z, Maxf( p5.z, Maxf( p6.z, Maxf( p7.z, p8.z ) ) ) ) ) ) );
	if (minz >= aabb.m_maxs.z || maxz <= aabb.m_mins.z) {
		return false;
	}

	return true;
}

bool DoSphereAndOBB3Overlap3D( Vec3 const& sphereCenter, float sphereRadius, OBB3 const& obb )
{
	Vec3 nearestPoint = GetNearestPointOnOBB3D( sphereCenter, obb );
	return IsPointInsideSphere3D( nearestPoint, sphereCenter, sphereRadius );
}

bool DoSphereAndAABB3Overlap3D( Vec3 const& sphereCenter, float sphereRadius, AABB3 const& aabb3 )
{
	Vec3 nearestPoint = GetNearestPointOnAABB3D( sphereCenter, aabb3 );
	return IsPointInsideSphere3D( nearestPoint, sphereCenter, sphereRadius );
}

bool DoAABB3AndZCylinderOverlap3D( AABB3 const& aabb3, Vec2 const& cylinderCenterXY, float cylinderRadius, float cylinderminZ, float cylindermaxZ )
{
	if (cylindermaxZ > aabb3.m_mins.z && cylinderminZ < aabb3.m_maxs.z
		&& DoDiscAndAABB2Overlap2D( cylinderCenterXY, cylinderRadius, AABB2( aabb3.m_mins, aabb3.m_maxs ) )) {
		return true;
	}
	return false;
}

bool DoZCylinderAndSphereOverlap3D( Vec2 const& cylinderCenterXY, float cylinderRadius, float cylinderminZ, float cylindermaxZ, Vec3 const& sphereCenter, float sphereRadius )
{
	Vec3 nearestPoint = GetNearestPointOnZCylinder3D( sphereCenter, cylinderCenterXY, cylinderRadius, cylinderminZ, cylindermaxZ );
	return IsPointInsideSphere3D( nearestPoint, sphereCenter, sphereRadius );
}

bool DoSphereAndPlaneOverlap3D( Vec3 const& sphereCenter, float sphereRadius, Plane3 const& plane )
{
	float altitude = plane.GetAltitudeOfPoint( sphereCenter );
	return sphereRadius > abs( altitude );
}

bool DoAABB3AndPlaneOverlap3D( AABB3 const& aabb3, Plane3 const& plane )
{
	// reference: unreal engine source code
	/*
	* If the point positively farthest to the plane is below the plane 
	* or negatively farthest to the plane is above the plane, then there is no intersection
	* So get the max and min point
	*/
	Vec3 planeRelativeMin, planeRelativeMax;
	float* planeRelativeMinPtr = (float*)&planeRelativeMin;
	float* planeRelativeMaxPtr = (float*)&planeRelativeMax;

	float const* aabbMinPtr = (float*)&aabb3.m_mins;
	float const* aabbMaxPtr = (float*)&aabb3.m_maxs;
	float const* planeNormalPtr = (float*)&plane.m_normal;

	for (int i = 0; i < 3; i++) {
		if (planeNormalPtr[i] > 0.f) {
			planeRelativeMinPtr[i] = aabbMinPtr[i];
			planeRelativeMaxPtr[i] = aabbMaxPtr[i];
		}
		else {
			planeRelativeMinPtr[i] = aabbMaxPtr[i];
			planeRelativeMaxPtr[i] = aabbMinPtr[i];
		}
	}

	float distanceMax = plane.GetAltitudeOfPoint( planeRelativeMax );
	float distanceMin = plane.GetAltitudeOfPoint( planeRelativeMin );
	if (distanceMax <= 0.f || distanceMin >= 0.f) {
		return false;
	}
	return true;
}

bool DoOBB3AndPlaneOverlap3D( OBB3 const& obb3, Plane3 const& plane )
{
	Plane3 tempPlane( plane.m_normal, plane.m_distanceFromOrigin );
	tempPlane.Translate( -obb3.m_center );
	tempPlane.m_normal = Vec3( DotProduct3D( plane.m_normal, obb3.m_iBasis ), DotProduct3D( plane.m_normal, obb3.m_jBasis ), DotProduct3D( plane.m_normal, obb3.m_kBasis ) );
	
	AABB3 localBox = AABB3( -obb3.m_halfDimensions, obb3.m_halfDimensions );
	return DoAABB3AndPlaneOverlap3D( localBox, tempPlane );
}

bool DoZCylinderAndPlaneOverlap3D( Vec2 const& cylinderCenterXY, float cylinderRadius, float cylinderminZ, float cylindermaxZ, Plane3 const& plane )
{
	// get the points on cylinder which are farthest to plane

	// first check plane Normal.z == 1 or -1
	if (abs( plane.m_normal.z - 1.f ) < 0.0001) {
		return plane.m_distanceFromOrigin < cylindermaxZ && plane.m_distanceFromOrigin > cylinderminZ;
	}
	else if (abs( plane.m_normal.z + 1.f ) < 0.0001) {
		return -plane.m_distanceFromOrigin < cylindermaxZ && -plane.m_distanceFromOrigin > cylinderminZ;
	}

	Vec3 planeRelativeMin, planeRelativeMax;
	// XY plane: Get the direction on XY plane
	Vec2 planeNormal2D = Vec2( plane.m_normal ).GetNormalized();

	planeRelativeMax.x = cylinderCenterXY.x + planeNormal2D.x * cylinderRadius;
	planeRelativeMax.y = cylinderCenterXY.y + planeNormal2D.y * cylinderRadius;
	planeRelativeMin.x = cylinderCenterXY.x - planeNormal2D.x * cylinderRadius;
	planeRelativeMin.y = cylinderCenterXY.y - planeNormal2D.y * cylinderRadius;

	if (plane.m_normal.z >= 0.f) {
		planeRelativeMax.z = cylindermaxZ;
		planeRelativeMin.z = cylinderminZ;
	}
	else {
		planeRelativeMax.z = cylinderminZ;
		planeRelativeMin.z = cylindermaxZ;
	}

	float distanceMax = plane.GetAltitudeOfPoint( planeRelativeMax );
	float distanceMin = plane.GetAltitudeOfPoint( planeRelativeMin );
	if (distanceMax <= 0.f || distanceMin >= 0.f) {
		return false;
	}
	return true;
}

Vec3 GetNearestPointOnAABB3D( Vec3 const& referencePosition, AABB3 const& aabb3 )
{
	return aabb3.GetNearestPoint( referencePosition );
}

Vec3 GetNearestPointOnSphere3D( Vec3 const& referencePosition, Vec3 const& sphereCenter, float sphereRadius )
{
	Vec3 CP = referencePosition - sphereCenter;
	return sphereCenter + CP.GetClamped( sphereRadius );
}

Vec3 GetNearestPointOnZCylinder3D( Vec3 const& referencePosition, Vec2 const& cylinderCenterXY, float cylinderRadius, float cylinderminZ, float cylindermaxZ )
{
	Vec3 retValue = GetNearestPointOnDisc2D( referencePosition, cylinderCenterXY, cylinderRadius );
	retValue.z = GetClamped( referencePosition.z, cylinderminZ, cylindermaxZ );
	return retValue;
}

Vec3 GetNearestPointOnPlane3D( Vec3 const& referencePosition, Plane3 const& plane )
{
	return plane.GetNearestPoint( referencePosition );
}

Vec3 GetNearestPointOnOBB3D( Vec3 const& referencePosition, OBB3 const& obb3 )
{
	return obb3.GetNearestPoint( referencePosition );
}

Vec3 GetNearestPointOnLineSegment3D( Vec3 const& referencePosition, Vec3 const& startPoint, Vec3 const& endPoint )
{
	Vec3 forwardVector = endPoint - startPoint;
	Vec3 startToRef = referencePosition - startPoint;
	Vec3 endToRef = referencePosition - endPoint;
	float dotProdForward = DotProduct3D( startToRef, forwardVector );
	if (dotProdForward <= 0.f) {
		return startPoint;
	}
	else if (DotProduct3D( endToRef, forwardVector ) >= 0.f) {
		return endPoint;
	}
	float inversedForwardLength = 1.f / forwardVector.GetLength();
	return startPoint + forwardVector * dotProdForward * inversedForwardLength * inversedForwardLength;

}

float GetPointDistanceToLineSegment3D( Vec3 const& referencePosition, Vec3 const& startPoint, Vec3 const& endPoint )
{
	Vec3 const closestPoint = GetNearestPointOnLineSegment3D( referencePosition, startPoint, endPoint );
	return (closestPoint - referencePosition).GetLength();
}

float GetPointDistanceToLineSegmentSquared3D( Vec3 const& referencePosition, Vec3 const& startPoint, Vec3 const& endPoint )
{
	Vec3 const closestPoint = GetNearestPointOnLineSegment3D( referencePosition, startPoint, endPoint );
	return (closestPoint - referencePosition).GetLengthSquared();
}

float ComputeCubicBezier1D( float A, float B, float C, float D, float t )
{
	float E = (1.f - t) * A + t * B;
	float F = (1.f - t) * B + t * C;
	float G = (1.f - t) * C + t * D;

	float H = (1.f - t) * E + t * F;
	float I = (1.f - t) * F + t * G;

	return (1.f - t) * H + t * I;
}

float ComputeQuinticBezier1D( float A, float B, float C, float D, float E, float F, float t )
{
	float AB = Interpolate( A, B, t );
	float BC = Interpolate( B, C, t );
	float CD = Interpolate( C, D, t );
	float DE = Interpolate( D, E, t );
	float EF = Interpolate( E, F, t );

	float AC = Interpolate( AB, BC, t );
	float BD = Interpolate( BC, CD, t );
	float CE = Interpolate( CD, DE, t );
	float DF = Interpolate( DE, EF, t );

	float AD = Interpolate( AC, BD, t );
	float BE = Interpolate( BD, CE, t );
	float CF = Interpolate( CE, DF, t );

	float AE = Interpolate( AD, BE, t );
	float BF = Interpolate( BE, CF, t );

	return Interpolate( AE, BF, t );
}

float SmoothStart2( float t )
{
	return t * t;
}

float SmoothStart3( float t )
{
	return t * t * t;
}

float SmoothStart4( float t )
{
	return (t * t) * (t * t);
}

float SmoothStart5( float t )
{
	return (t * t) * (t * t) * t;
}

float SmoothStart6( float t )
{
	return (t * t) * (t * t) * (t * t);
}

float SmoothStop2( float t )
{
	float s = 1.f - t;
	return 1.f - s * s;
}

float SmoothStop3( float t )
{
	float s = 1.f - t;
	return 1.f - s * s * s;
}

float SmoothStop4( float t )
{
	float s = 1.f - t;
	return 1.f - (s * s) * (s * s);
}

float SmoothStop5( float t )
{
	float s = 1.f - t;
	return 1.f - (s * s) * (s * s) * s;
}

float SmoothStop6( float t )
{
	float s = 1.f - t;
	return 1.f - (s * s) * (s * s) * (s * s);
}

float SmoothStep3( float t )
{
	return (1.f - t) * SmoothStart2( t ) + t * SmoothStop2( t );
	//return 3.f * SmoothStart2( t ) - 2.f * SmoothStart3( t );
}

float SmoothStep5( float t )
{
	return ComputeQuinticBezier1D( 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, t );
}

float Hesitate3( float t )
{
	return ComputeCubicBezier1D( 0.f, 1.f, 0.f, 1.f, t );
}

float Hesitate5( float t )
{
//	return (1.f - t) * SmoothStop4( t ) + t * SmoothStart4( t );
	return ComputeQuinticBezier1D( 0.f, 1.f, 0.f, 1.f, 0.f, 1.f, t );
}

bool DoAABB2sOverlap2D( AABB2 const& first, AABB2 const& second )
{
	float aMinX = first.m_mins.x;
	float aMaxX = first.m_maxs.x;
	float aMinY = first.m_mins.y;
	float aMaxY = first.m_maxs.y;
	float bMinX = second.m_mins.x;
	float bMaxX = second.m_maxs.x;
	float bMinY = second.m_mins.y;
	float bMaxY = second.m_maxs.y;

	if (aMaxX > bMinX && bMaxX > aMinX && aMaxY > bMinY && bMaxY > aMinY) {
		return true;
	}
	return false;
}

bool DoDiscAndAABB2Overlap2D( Vec2 const& discCenter, float radius, AABB2 const& aabb2 )
{
	Vec2 nearestPoint = GetNearestPointOnAABB2D( discCenter, aabb2 );
	return IsPointInsideDisc2D( nearestPoint, discCenter, radius );
}

bool DoDiscOverlapDirectedSector2D( Vec2 const& discCenter, float discRadius, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius )
{
	Vec2 nearestPointOnSector = GetNearestPointOnDirectedSector2D( discCenter, sectorTip, sectorForwardNormal, sectorApertureDegrees, sectorRadius );
	return IsPointInsideDisc2D( nearestPointOnSector, discCenter, discRadius );
}

bool DoDiscAndSectorOverlap2D( Vec2 const& discCenter, float discRadius, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius )
{
	Vec2 nearestPointOnSector = GetNearestPointOnOrientedSector2D( discCenter, sectorTip, sectorForwardDegrees, sectorApertureDegrees, sectorRadius );
	return IsPointInsideDisc2D( nearestPointOnSector, discCenter, discRadius );
}

bool DoDiscAndOBB2Overlap2D( Vec2 const& discCenter, float radius, OBB2 const& obb2 )
{
	Vec2 nearestPointOnOBB2 = GetNearestPointOnOBB2D( discCenter, obb2 );
	return IsPointInsideDisc2D( nearestPointOnOBB2, discCenter, radius );
}

bool DoDiscAndCapsuleOverlap2D( Vec2 const& discCenter, float discRadius, Vec2 const& boneStartPoint, Vec2 const& boneEndPoint, float capsuleRadius )
{
	Vec2 nearestPointOnCapsule = GetNearestPointOnCapsule2D( discCenter, boneStartPoint, boneEndPoint, capsuleRadius );
	return IsPointInsideDisc2D( nearestPointOnCapsule, discCenter, discRadius );
}

float GetPointDistanceToLineSegment2D( Vec2 const& referencePosition, Vec2 const& startPoint, Vec2 const& endPoint )
{
	Vec2 const closestPoint = GetNearestPointOnLineSegment2D( referencePosition, startPoint, endPoint );
	return (closestPoint - referencePosition).GetLength();
}

float GetPointDistanceToLineSegmentSquared2D( Vec2 const& referencePosition, Vec2 const& startPoint, Vec2 const& endPoint )
{
	Vec2 const closestPoint = GetNearestPointOnLineSegment2D( referencePosition, startPoint, endPoint );
	return (closestPoint - referencePosition).GetLengthSquared();
}

float GetPointDistanceToInfiniteLine2D( Vec2 const& referencePosition, Vec2 const& startPoint, Vec2 const& directionNormal )
{
	float projectionLength = DotProduct2D( (referencePosition - startPoint), directionNormal );
	Vec2 cloestPoint = startPoint + directionNormal * projectionLength;
	return (cloestPoint - referencePosition).GetLength();
}

float GetPointDistanceToInfiniteLineSquared2D( Vec2 const& referencePosition, Vec2 const& startPoint, Vec2 const& directionNormal )
{
	float projectionLength = DotProduct2D( (referencePosition - startPoint), directionNormal );
	Vec2 cloestPoint = startPoint + directionNormal * projectionLength;
	return (cloestPoint - referencePosition).GetLengthSquared();
}

bool IsPointInsideDisc2D( Vec2 const& point, Vec2 const& discCenter, float discRadius )
{
	return GetDistanceSquared2D( discCenter, point ) < discRadius * discRadius;
}

bool IsPointInsideOrientedSector2D( Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius )
{
	Vec2 transformedPoint = point - sectorTip;
	float degrees = Atan2Degrees( transformedPoint.y, transformedPoint.x );
	float length = transformedPoint.GetLength();
	if (length > sectorRadius) {
		return false;
	}
	if (abs(GetShortestAngularDispDegrees(sectorForwardDegrees, degrees)) >= sectorApertureDegrees * 0.5f) {
		return false;
	}
	return true;
}

bool IsPointInsideDirectedSector2D( Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius )
{
	Vec2 transformedPoint = point - sectorTip;
	float length = transformedPoint.GetLength();
	if (length > sectorRadius) {
		return false;
	}
	if (GetAngleDegreesBetweenVectors2D(transformedPoint, sectorForwardNormal) >= sectorApertureDegrees * 0.5f) {
		return false;
	}
	return true;
}

bool IsPointInsideAABB2D( Vec2 const& point, AABB2 const& aabbBox )
{
	return aabbBox.IsPointInside( point );
}

bool IsPointInsideCapsule2D( Vec2 const& point, Vec2 const& boneStartPoint, Vec2 const& boneEndPoint, float radius )
{
	Vec2 forwardVector = (boneEndPoint - boneStartPoint);
	float length = forwardVector.GetLength();
	Vec2 iBasisNormal = forwardVector / length;
	Vec2 localPoint = (point - boneStartPoint).GetRotatedNewBasis( GetInversedOrthonormaliBasis( iBasisNormal ) );
	if (IsPointInsideAABB2D( localPoint, AABB2( Vec2( 0.f, -radius ), Vec2( length, radius ) ) )) {
		return true;
	}
	else if (IsPointInsideDisc2D( localPoint, Vec2( 0.f, 0.f ), radius )) {
		return true;
	}
	else if (IsPointInsideDisc2D( localPoint, Vec2( length, 0.f ), radius )) {
		return true;
	}
	return false;
}

bool IsPointInsideOBB2D( Vec2 const& point, OBB2 const& obbBox )
{
	return obbBox.IsPointInside( point );
}

bool IsPointInsideConvexHull2D( Vec2 const& point, ConvexHull2 const& convexHull )
{
	for (int i = 0; i < (int)convexHull.m_boundingPlanes.size(); ++i) {
		if (convexHull.m_boundingPlanes[i].GetAltitudeOfPoint( point ) > 0.f) {
			return false;
		}
	}
	return true;
}

// ref:https://stackoverflow.com/questions/2049582/how-to-determine-if-a-point-is-in-a-2d-triangle
bool IsPointInsideTriangle2D( Vec2 const& point, Vec2 const& triVert1, Vec2 const& triVert2, Vec2 const& triVert3 )
{
	float d1, d2, d3;
	bool has_neg, has_pos;

	auto sign = []( Vec2 const& p1, Vec2 const& p2, Vec2 const& p3 )
		{
			return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
		};

	d1 = sign( point, triVert1, triVert2 );
	d2 = sign( point, triVert2, triVert3 );
	d3 = sign( point, triVert3, triVert1 );

	has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
	has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

	return !(has_neg && has_pos);
}

Vec2 GetPlaneIntersection2D( Plane2 const& plane1, Plane2 const& plane2 )
{
	if (abs( CrossProduct2D( plane1.m_normal, plane2.m_normal ) ) < 0.0001f) {
		ERROR_RECOVERABLE( "Cannot get the intersection point if two planes are paralleled with each other" );
		return Vec2();
	}
	float divider = plane1.m_normal.x * plane2.m_normal.y - plane2.m_normal.x * plane1.m_normal.y;
	return Vec2((plane1.m_distanceFromOrigin * plane2.m_normal.y - plane2.m_distanceFromOrigin * plane1.m_normal.y)
			/	divider,
				(plane2.m_distanceFromOrigin * plane1.m_normal.x - plane1.m_distanceFromOrigin * plane2.m_normal.x) 
			/	divider);
}

Vec3 GetBarycentricCoordinate( Vec2 const& point, Vec2 const& triVert1, Vec2 const& triVert2, Vec2 const& triVert3 )
{
	float divider = (triVert1.x - triVert3.x) * (triVert2.y - triVert3.y) - (triVert1.y - triVert3.y) * (triVert2.x - triVert3.x);
	float a = -((triVert2.x - triVert3.x) * (point.y - triVert3.y) - (triVert2.y - triVert3.y) * (point.x - triVert3.x)) / divider;
	float b = ((triVert1.x - triVert3.x) * (point.y - triVert3.y) - (triVert1.y - triVert3.y) * (point.x - triVert3.x)) / divider;
	float c = 1.f - a - b;
	return Vec3( a, b, c );
}

Vec2 GetNearestPointOnDisc2D( Vec2 const& referencePosition, Vec2 const& discCenter, float discRadius )
{
	Vec2 forwardVector = referencePosition - discCenter;
	float forwardVectorLength = forwardVector.GetLength();
	// point in disc
	if (forwardVectorLength < discRadius) {
		return referencePosition;
	}
	return discCenter + forwardVector * (discRadius / forwardVectorLength);
}

Vec2 GetNearestPointOnCircle2D( Vec2 const& referencePosition, Vec2 const& circleCenter, float circleRadius )
{
	Vec2 forwardVector = referencePosition - circleCenter;
	float forwardVectorLength = forwardVector.GetLength();
	return circleCenter + forwardVector * (circleRadius / forwardVectorLength);
}

Vec2 GetNearestPointOnAABB2D( Vec2 const& referencePosition, AABB2 const& aabbBox )
{
	return aabbBox.GetNearestPoint( referencePosition );
}

Vec2 GetNearestPointOnInfiniteLine2D( Vec2 const& referencePosition, Vec2 const& point1, Vec2 const& point2 )
{
	Vec2 forwardVector = point2 - point1;
	Vec2 point1ToRef = referencePosition - point1;
	float inversedForwardLength = 1.f / forwardVector.GetLength();
	return point1 + forwardVector * DotProduct2D( point1ToRef, forwardVector ) * inversedForwardLength * inversedForwardLength;
}

Vec2 GetNearestPointOnLineSegment2D( Vec2 const& referencePosition, Vec2 const& startPoint, Vec2 const& endPoint )
{
	Vec2 forwardVector = endPoint - startPoint;
	Vec2 startToRef = referencePosition - startPoint;
	Vec2 endToRef = referencePosition - endPoint;
	float dotProdForward = DotProduct2D( startToRef, forwardVector );
	if (dotProdForward <= 0.f) {
		return startPoint;
	}
	else if (DotProduct2D( endToRef, forwardVector ) >= 0.f) {
		return endPoint;
	}
	float inversedForwardLength = 1.f / forwardVector.GetLength();
	return startPoint + forwardVector * dotProdForward * inversedForwardLength * inversedForwardLength;

	/*
	Vec2 forwardVector = endPoint - startPoint;
	float forwardLength = forwardVector.GetLength();
	Vec2 iBasisNormal = forwardVector / forwardLength;
	Vec2 localPos = (referencePosition - startPoint).GetRotatedNewBasis( GetInversedOrthometriciBasis( iBasisNormal ) );
	if (localPos.x <= 0.f) {
		return startPoint;
	}
	else if (localPos.x > forwardLength) {
		return endPoint;
	}
	return Vec2( localPos.x, 0 ).GetRotatedNewBasis( iBasisNormal ) + startPoint;
	*/
}

Vec2 GetNearestPointOnCapsule2D( Vec2 const& referencePosition, Vec2 const& boneStartPoint, Vec2 const& boneEndPoint, float radius )
{
	Vec2 forwardVector = (boneEndPoint - boneStartPoint);
	float length = forwardVector.GetLength();
	Vec2 iBasisNormal = forwardVector / length;
	Vec2 localPoint = (referencePosition - boneStartPoint).GetRotatedNewBasis( GetInversedOrthonormaliBasis( iBasisNormal ) );
	if (localPoint.x < 0.f) {
		return GetNearestPointOnDisc2D( localPoint, Vec2( 0.f, 0.f ), radius ).GetRotatedNewBasis( iBasisNormal ) + boneStartPoint;
	}
	else if (localPoint.x > length) {
		return GetNearestPointOnDisc2D( localPoint, Vec2( length, 0.f ), radius ).GetRotatedNewBasis( iBasisNormal ) + boneStartPoint;
	}
	else {
		if (localPoint.y > radius) {
			return Vec2( localPoint.x, radius ).GetRotatedNewBasis( iBasisNormal ) + boneStartPoint;
		}
		else if (localPoint.y < -radius) {
			return Vec2( localPoint.x, -radius ).GetRotatedNewBasis( iBasisNormal ) + boneStartPoint;
		}
		else {
			return referencePosition;
		}
	}
}

Vec2 GetNearestPointOnOBB2D( Vec2 const& referencePosition, OBB2 const& obbBox )
{
	return obbBox.GetNearestPoint( referencePosition );
}

Vec2 GetNearestPointOnOrientedSector2D( Vec2 const& referencePosition, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius )
{
	Vec2 transformedPoint = referencePosition - sectorTip;
	float degrees = Atan2Degrees( transformedPoint.y, transformedPoint.x );
	float length = transformedPoint.GetLength();
	float offsetDegrees = GetShortestAngularDispDegrees( sectorForwardDegrees, degrees );
	if (length < sectorRadius && abs( offsetDegrees ) <= sectorApertureDegrees * 0.5f) {
		return referencePosition;
	}
	if (abs( offsetDegrees ) <= sectorApertureDegrees * 0.5f) {
		return GetNearestPointOnDisc2D( referencePosition, sectorTip, sectorRadius );
	}
	if (offsetDegrees < 0.f) {
		return GetNearestPointOnLineSegment2D( referencePosition, sectorTip, sectorTip + Vec2::MakeFromPolarDegrees( sectorForwardDegrees, sectorRadius ).GetRotatedDegrees( -sectorApertureDegrees * 0.5f ) );
	}
	else {
		return GetNearestPointOnLineSegment2D( referencePosition, sectorTip, sectorTip + Vec2::MakeFromPolarDegrees( sectorForwardDegrees, sectorRadius ).GetRotatedDegrees( sectorApertureDegrees * 0.5f ) );
	}
}

Vec2 GetNearestPointOnDirectedSector2D( Vec2 const& referencePosition, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius )
{
	return GetNearestPointOnOrientedSector2D( referencePosition, sectorTip, sectorForwardNormal.GetOrientationDegrees(), sectorApertureDegrees, sectorRadius );
}

bool PushDiscOutOfFixedPoint2D( Vec2& mobileDiscCenter, float discRadius, Vec2 const& fixedPoint )
{
	if (!IsPointInsideDisc2D( fixedPoint, mobileDiscCenter, discRadius )) {
		return false;
	}
	if (fixedPoint == mobileDiscCenter) {
		return false;
	}
	Vec2 forwardVector = fixedPoint - GetNearestPointOnCircle2D( fixedPoint, mobileDiscCenter, discRadius );
	mobileDiscCenter += forwardVector;
	return true;
	/*
	Vec2 forwardVector = mobileDiscCenter - fixedPoint;
	float forwardVectorLength = forwardVector.GetLength();
	if (forwardVectorLength > discRadius) {
		return false;
	}
	mobileDiscCenter += (forwardVector.GetNormalized() * (discRadius - forwardVectorLength));
	return true;
	*/
}

bool PushDiscOutOfFixedDisc2D( Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedDiscCenter, float fixedDiscRadius )
{
	if (GetDistanceSquared2D( fixedDiscCenter, mobileDiscCenter ) > (mobileDiscRadius + fixedDiscRadius) * (mobileDiscRadius + fixedDiscRadius)) {
		return false;
	}
	if (mobileDiscCenter == fixedDiscCenter) {
		return false;
	}
	Vec2 forwardVector = GetNearestPointOnCircle2D( mobileDiscCenter, fixedDiscCenter, fixedDiscRadius ) 
					   - GetNearestPointOnCircle2D( fixedDiscCenter, mobileDiscCenter, mobileDiscRadius );
	mobileDiscCenter += forwardVector;
	return true;
}

bool PushDiscsOutOfEachOther2D( Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius )
{
	if (GetDistanceSquared2D( aCenter, bCenter ) > (aRadius + bRadius) * (aRadius + bRadius)) {
		return false;
	}
	if (aCenter == bCenter) {
		return false;
	}
	Vec2 forwardVector = GetNearestPointOnCircle2D( aCenter, bCenter, bRadius )
					   - GetNearestPointOnCircle2D( bCenter, aCenter, aRadius );
	aCenter += (forwardVector * 0.5f);
	bCenter -= (forwardVector * 0.5f);
	return true;
}

bool PushDiscOutOfFixedAABB2D( Vec2& mobileDiscCenter, float discRadius, AABB2 const& fixedBox )
{
	if (mobileDiscCenter == fixedBox.GetCenter()) {
		return false;
	}
	AABB2 largerFixedBox1( fixedBox.m_mins - Vec2( discRadius, 0 ), fixedBox.m_maxs + Vec2( discRadius, 0 ) );
	AABB2 largerFixedBox2( fixedBox.m_mins - Vec2( 0, discRadius ), fixedBox.m_maxs + Vec2( 0, discRadius ) );

	if (largerFixedBox1.IsPointInside( mobileDiscCenter ) || largerFixedBox2.IsPointInside( mobileDiscCenter )) 
	{
		float moveLeftDist = mobileDiscCenter.x + discRadius - fixedBox.m_mins.x;
		float moveRightDist = fixedBox.m_maxs.x - (mobileDiscCenter.x - discRadius);
		float moveUpDist = fixedBox.m_maxs.y - (mobileDiscCenter.y - discRadius);
		float moveDownDist = mobileDiscCenter.y + discRadius - fixedBox.m_mins.y;
		if (moveLeftDist == Minf( Minf( Minf( moveLeftDist, moveRightDist ), moveUpDist ), moveDownDist )) {
			mobileDiscCenter -= Vec2( moveLeftDist, 0 );
		}
		else if (moveRightDist == Minf( Minf( Minf( moveLeftDist, moveRightDist ), moveUpDist ), moveDownDist )) {
			mobileDiscCenter += Vec2( moveRightDist, 0 );
		}
		else if (moveUpDist == Minf( Minf( Minf( moveLeftDist, moveRightDist ), moveUpDist ), moveDownDist )) {
			mobileDiscCenter += Vec2( 0, moveUpDist );
		}
		else {
			mobileDiscCenter -= Vec2( 0, moveDownDist );
		}
		return true;
	}
	else if (GetDistanceSquared2D( mobileDiscCenter, fixedBox.m_mins ) < discRadius * discRadius) {
		mobileDiscCenter += (fixedBox.m_mins - GetNearestPointOnCircle2D( fixedBox.m_mins, mobileDiscCenter, discRadius ));
		return true;
	}
	else if (GetDistanceSquared2D( mobileDiscCenter, fixedBox.m_maxs ) < discRadius * discRadius) {
		mobileDiscCenter += (fixedBox.m_maxs - GetNearestPointOnCircle2D( fixedBox.m_maxs, mobileDiscCenter, discRadius ));
		return true;
	}
	else if (GetDistanceSquared2D( mobileDiscCenter, Vec2( fixedBox.m_mins.x, fixedBox.m_maxs.y ) ) < discRadius * discRadius) {
		mobileDiscCenter += (Vec2( fixedBox.m_mins.x, fixedBox.m_maxs.y ) - GetNearestPointOnCircle2D( Vec2( fixedBox.m_mins.x, fixedBox.m_maxs.y ), mobileDiscCenter, discRadius ));
		return true;
	}
	else if (GetDistanceSquared2D( mobileDiscCenter, Vec2( fixedBox.m_maxs.x, fixedBox.m_mins.y ) ) < discRadius * discRadius) {
		mobileDiscCenter += (Vec2( fixedBox.m_maxs.x, fixedBox.m_mins.y ) - GetNearestPointOnCircle2D( Vec2( fixedBox.m_maxs.x, fixedBox.m_mins.y ), mobileDiscCenter, discRadius ));
		return true;
	}
	return false;
}

bool BounceDiscOutOfEachOther2D( Vec2& aCenter, float aRadius, Vec2& aVelocity, float aElasticity, Vec2& bCenter, float bRadius, Vec2& bVelocity, float bElasticity )
{
	Vec2 normalAtoB = bCenter - aCenter;
	float squaredLength = normalAtoB.GetLengthSquared();

	// Do discs overlap
	if (squaredLength >= (aRadius + bRadius) * (aRadius + bRadius) || squaredLength == 0.f) {
		return false;
	}

	float length = sqrtf( squaredLength );
	float inversedLength = 1.f / length;
	normalAtoB = normalAtoB * inversedLength;

	float dotANormal = DotProduct2D( normalAtoB, aVelocity );
	float dotBNormal = DotProduct2D( normalAtoB, bVelocity );

	// Push disc out of each other
	Vec2 nearestPointFromAonB = bCenter - bRadius * normalAtoB;
	Vec2 nearestPointFromBonA = aCenter + aRadius * normalAtoB;
	Vec2 difference = nearestPointFromAonB - nearestPointFromBonA;
	aCenter += difference * 0.5f;
	bCenter -= difference * 0.5f;

	if (dotANormal <= dotBNormal)
	{
		return false;
	}
	// accept
	Vec2 velocityAgreeOnNormalDirA = dotANormal * normalAtoB;
	Vec2 velocityAgreeOnNormalDirB = dotBNormal * normalAtoB;
	Vec2 velocityIndependentA = aVelocity - velocityAgreeOnNormalDirA;
	Vec2 velocityIndependentB = bVelocity - velocityAgreeOnNormalDirB;

	// Bounce the velocity of disc A
	aVelocity = velocityIndependentA + aElasticity * bElasticity * velocityAgreeOnNormalDirB;

	// Bounce the velocity of disc B
	bVelocity = velocityIndependentB + aElasticity * bElasticity * velocityAgreeOnNormalDirA;

	return true;
}

bool BounceDiscOutOfFixedPoint2D( Vec2& discCenter, float discRadius, Vec2& discVelocity, float discElasticity, Vec2 const& fixedPoint, float pointElasticity )
{
	Vec2 normal = discCenter - fixedPoint;
	float squaredLength = normal.GetLengthSquared();

	// Do disc overlap the point?
	if (squaredLength >= discRadius * discRadius || squaredLength == 0.f) {
		return false;
	}

	float length = sqrtf( squaredLength );
	float inversedLength = 1.f / length;
	normal = normal * inversedLength;

	// Push disc out of the fixed point
	discCenter += normal * (discRadius - length);

	// velocity not agree?
	if (DotProduct2D( normal, discVelocity ) > 0.f) {
		return false;
	}

	// Bounce the velocity
	Vec2 velocityAgreeOnNormalDir = DotProduct2D( normal, discVelocity ) * normal;
	discVelocity = discVelocity - velocityAgreeOnNormalDir - discElasticity * pointElasticity * velocityAgreeOnNormalDir;

	return true;
}

bool BounceDiscOutOfFixedDisc2D( Vec2& mobileCenter, float mobileRadius, Vec2& mobileVelocity, float mobileElasticity, Vec2 const& fixedCenter, float fixedRadius, float fixedElasticity )
{
	Vec2 point = GetNearestPointOnDisc2D( mobileCenter, fixedCenter, fixedRadius );
	return BounceDiscOutOfFixedPoint2D( mobileCenter, mobileRadius, mobileVelocity, mobileElasticity, point, fixedElasticity );
}

bool BounceDiscOutOfFixedOBB2D( Vec2& mobileCenter, float mobileRadius, Vec2& mobileVelocity, float mobileElasticity, OBB2 const& fixedBox, float fixedElasticity )
{
	Vec2 point = GetNearestPointOnOBB2D( mobileCenter, fixedBox );
	return BounceDiscOutOfFixedPoint2D( mobileCenter, mobileRadius, mobileVelocity, mobileElasticity, point, fixedElasticity );
}

bool BounceDiscOutOfFixedCapsule2D( Vec2& mobileCenter, float mobileRadius, Vec2& mobileVelocity, float mobileElasticity, Vec2 const& fixedBoneStart, Vec2 const& fixedBoneEnd, float fixedRadius, float fixedElasticity )
{
	Vec2 point = GetNearestPointOnCapsule2D( mobileCenter, fixedBoneStart, fixedBoneEnd, fixedRadius );
	return BounceDiscOutOfFixedPoint2D( mobileCenter, mobileRadius, mobileVelocity, mobileElasticity, point, fixedElasticity );
}

bool IsPointInsideSphere3D( Vec3 const& point, Vec3 const& center, float radius )
{
	return GetDistanceSquared3D( center, point ) < radius * radius;
}

bool IsPointInsideAABB3D( Vec3 const& point, AABB3 const& aabb3 )
{
	return aabb3.IsPointInside( point );
}

bool IsPointInsideOBB3D( Vec3 const& point, OBB3 const& obb3 )
{
	return obb3.IsPointInside( point );
}

bool DoAABB3sOverlap3D( AABB3 const& first, AABB3 const& second )
{
	float aMinX = first.m_mins.x;
	float aMaxX = first.m_maxs.x;
	float aMinY = first.m_mins.y;
	float aMaxY = first.m_maxs.y;
	float aMinZ = first.m_mins.z;
	float aMaxZ = first.m_maxs.z;
	float bMinX = second.m_mins.x;
	float bMaxX = second.m_maxs.x;
	float bMinY = second.m_mins.y;
	float bMaxY = second.m_maxs.y;
	float bMinZ = second.m_mins.z;
	float bMaxZ = second.m_maxs.z;

	if (aMaxX > bMinX && bMaxX > aMinX && aMaxY > bMinY && bMaxY > aMinY && aMaxZ > bMinZ && bMaxZ > aMinZ) {
		return true;
	}
	return false;
}

void TransformPosition2D( Vec2& posToTransform, float uniformScale, float rotationDegrees, Vec2 const& translation )
{
	Vec2 iBasis = Vec2( CosDegrees( rotationDegrees ) * uniformScale, SinDegrees( rotationDegrees ) * uniformScale );
	Vec2 jBasis = Vec2( -iBasis.y, iBasis.x );
	TransformPosition2D( posToTransform, iBasis, jBasis, translation );
	/*float dist = sqrtf(posToTransform.x * posToTransform.x + posToTransform.y * posToTransform.y);
	dist *= uniformScale;
	float thetaRadians = atan2f( posToTransform.y, posToTransform.x );
	thetaRadians += ConvertDegreesToRadians( rotationDegrees );
	posToTransform.x = translation.x + (dist * cosf( thetaRadians ));
	posToTransform.y = translation.y + (dist * sinf( thetaRadians ));*/
}

/*
 ix jx tx     px
 iy jy ty  *  py = (ix * px + jx * py + tx, iy * px + jy * py + ty, 1)
 0  0  1      1
*/
void TransformPosition2D( Vec2& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation )
{
	posToTransform = iBasis * posToTransform.x + jBasis * posToTransform.y + translation;
}

void TransformPositionXY3D( Vec3& posToTransform, float uniformScale, float rotationDegrees, Vec2 const& translation )
{
	Vec2 iBasis = Vec2( CosDegrees( rotationDegrees ) * uniformScale, SinDegrees( rotationDegrees ) * uniformScale );
	Vec2 jBasis = Vec2( -iBasis.y, iBasis.x );
	TransformPositionXY3D( posToTransform, iBasis, jBasis, translation );
	/*float distXY = sqrtf(posToTransform.x * posToTransform.x + posToTransform.y * posToTransform.y);
	distXY *= uniformScale;
	float thetaRadians = atan2f( posToTransform.y, posToTransform.x );
	thetaRadians += ConvertDegreesToRadians(rotationDegrees);
	posToTransform.x = translation.x + (distXY * cosf( thetaRadians ));
	posToTransform.y = translation.y + (distXY * sinf( thetaRadians ));*/
}

void TransformPositionXY3D( Vec3& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation )
{
	//float newX = posToTransform.x * iBasis.x + posToTransform.y * jBasis.x;
	//float newY = posToTransform.x * iBasis.y + posToTransform.y * jBasis.y;
	Vec2 posToTransform2D = iBasis * posToTransform.x + jBasis * posToTransform.y + translation;
	posToTransform = Vec3( posToTransform2D.x, posToTransform2D.y, posToTransform.z );
}

Mat44 const GetBillboardMatrix( BillboardType billboardType, Mat44 const& cameraMatrix, Vec3 const& billboardPosition, Vec2 const& billboardScale /*= Vec2( 1.f, 1.f ) */ )
{
	Mat44 retMat;
	if (billboardType == BillboardType::FULL_CAMERA_FACING) {
		Vec3 forwardNormal = (cameraMatrix.GetTranslation3D() - billboardPosition).GetNormalized();
		Vec3 leftNormal;
		if (forwardNormal == Vec3( 0.f, 0.f, 1.f ) || forwardNormal == Vec3( 0.f, 0.f, -1.f )) {
			leftNormal = CrossProduct3D( Vec3( 0.f, 1.f, 0.f ), forwardNormal );
		}
		else {
			leftNormal = CrossProduct3D( Vec3( 0.f, 0.f, 1.f ), forwardNormal ).GetNormalized();
		}
		Vec3 upNormal = CrossProduct3D( forwardNormal, leftNormal );
		retMat.SetIJK3D( forwardNormal, leftNormal * billboardScale.x, upNormal * billboardScale.y );
		retMat.SetTranslation3D( billboardPosition );
		return retMat;
	}
	else if (billboardType == BillboardType::FULL_CAMERA_OPPOSING) {
		retMat.SetIJK3D( -cameraMatrix.GetIBasis3D(), -cameraMatrix.GetJBasis3D() * billboardScale.x, cameraMatrix.GetKBasis3D() * billboardScale.y );
		retMat.SetTranslation3D( billboardPosition );
		return retMat;
	}
	else if (billboardType == BillboardType::WORLD_UP_CAMERA_FACING) {
		Vec3 cameraPosition = cameraMatrix.GetTranslation3D();
		cameraPosition.z = billboardPosition.z;
		Vec3 forwardNormal = (cameraPosition - billboardPosition).GetNormalized();
		//if (forwardNormal == Vec3( 0.f, 0.f, 1.f ) || forwardNormal == -Vec3( 0.f, 0.f, 1.f )) {
		// should see nothing
		//}
		//else {
		Vec3 leftNormal = CrossProduct3D( Vec3( 0.f, 0.f, 1.f ), forwardNormal ).GetNormalized();
		//}
		retMat.SetIJK3D( forwardNormal, leftNormal * billboardScale.x, Vec3( 0.f, 0.f, 1.f ) * billboardScale.y );
		retMat.SetTranslation3D( billboardPosition );
	}
	else if (billboardType == BillboardType::WORLD_UP_CAMERA_OPPOSING) {
		Vec3 forwardNormal = Vec3( -cameraMatrix.GetIBasis3D().x, -cameraMatrix.GetIBasis3D().y, 0.f ).GetNormalized();
		Vec3 leftNormal = Vec3( -cameraMatrix.GetJBasis3D().x, -cameraMatrix.GetJBasis3D().y, 0.f ).GetNormalized();
		retMat.SetIJK3D( forwardNormal, leftNormal * billboardScale.x, Vec3( 0.f, 0.f, 1.f ) * billboardScale.y );
		retMat.SetTranslation3D( billboardPosition );
	}
	return retMat;
}

Vec3 RotateVectorAroundAxis3D( Vec3 const& vecToTotate, Vec3 const& axisNormal, float rotationDegrees )
{
	float sinTheta = SinDegrees( rotationDegrees );
	float cosTheta = CosDegrees( rotationDegrees );
	return vecToTotate * cosTheta 
		+ CrossProduct3D( axisNormal, vecToTotate ) * sinTheta 
		+ DotProduct3D( axisNormal, vecToTotate ) * (1 - cosTheta) * axisNormal;
}

int IntPow( int x, unsigned int p )
{
	if (p == 0) return 1;
	if (p == 1) return x;

	int tmp = IntPow( x, p / 2 );
	if (p % 2 == 0) return tmp * tmp;
	else return x * tmp * tmp;
}
