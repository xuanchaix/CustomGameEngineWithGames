#pragma once
#include "Engine/Math/Vec2.hpp"

struct Vec3;
struct Vec4;
struct IntVec2;
struct AABB2;
struct AABB3;
struct OBB2;
struct Mat44;
struct FloatRange;
struct Plane2;
struct Plane3;
struct OBB3;
struct ConvexHull2;
struct ConvexPoly2;

/// const values
constexpr float PI = 3.14159265359f;
constexpr float INV_SQRT_2 = 0.70710678118654752440084436210485f;

/// Enum types of billboard 
enum class BillboardType {
	NONE = -1,
	WORLD_UP_CAMERA_FACING,
	WORLD_UP_CAMERA_OPPOSING,
	FULL_CAMERA_FACING,
	FULL_CAMERA_OPPOSING,
	COUNT
};

//---------------------------
// Basic Math

/// Get the absolute value of float
float Absf( float value );
/// Get the minimal value of two floats
float Minf( float a, float b );
/// Get the maximal value of two floats
float Maxf( float a, float b );

/// Normalize byte to float from 0 to 1
float NormalizeByte( unsigned char byte );
/// Return a float from 0 to 1 to a unsigned char byte
unsigned char DenormalizeByte( float floatNum );

/// Return true if two float range has any intersection
bool DoFloatRangeOverlap( FloatRange const& f1, FloatRange const& f2 );

//---------------------------
// Clamp and lerp

/// Clamp the value into the two bounds
float GetClamped( float value, float minValue, float maxValue ); // -
/// Clamp the value into the two bounds
int GetClamped( int value, int minValue, int maxValue );
/// Clamp the value into 0 to 1
float GetClampedZeroToOne( float value ); // -
/// Get the value between start and end by faction
float Interpolate( float start, float end, float fractionTowardEnd ); // -
/// Get the value between start and end by faction
Vec2 Interpolate( Vec2 const& start, Vec2 const& end, float fractionTowardEnd );
/// Get the value between start and end by faction
Vec3 Interpolate( Vec3 const& start, Vec3 const& end, float fractionTowardEnd );
/// Get the value between start and end by faction
Vec4 Interpolate( Vec4 const& start, Vec4 const& end, float fractionTowardEnd );
/// Get the faction of the value between start and end
float GetFractionWithinRange( float value, float rangeStart, float rangeEnd );
/// Map the value from range in to range out
float RangeMap( float inValue, float inStart, float inEnd, float outStart, float outEnd ); // -
/// Clamp and Map the value from range in to range out
float RangeMapClamped( float inValue, float inStart, float inEnd, float outStart, float outEnd ); // -
/// Round down a float to a int
int RoundDownToInt( float value );
/// Round the float to a nearest integer
int RoundToInt( float value );

//----------------------------
// Angle utilities

/// Convert degrees to radians
float ConvertDegreesToRadians( float degrees ); // -
/// Convert degrees to radians
float ConvertRadiansToDegrees( float radians ); // -
/// Calculate the cosine value of a degree value
float CosDegrees( float degrees ); // -
/// Calculate the sine value of a degree value
float SinDegrees( float degrees ); // -
/// Calculate the cosine value of a radian value
float CosRadians( float radians );
/// Calculate the sine value of a radian value
float SinRadians( float radians );
/// Calculate the orientation degrees by position 
float Atan2Degrees( float y, float x ); // -
/// Calculate the orientation degrees by position
float Atan2Radians( float y, float x );
/// Calculate the shortest displacement degrees from start to end
float GetShortestAngularDispDegrees( float startDegrees, float endDegrees ); // -
// Has max delta degrees to turn, try best to turn to goal degrees from current degrees
float GetTurnedTowardDegrees( float currentDegrees, float goalDegrees, float maxDeltaDegrees ); // -
// Get the angular degrees between two vectors 
float GetAngleDegreesBetweenVectors2D( Vec2 const& a, Vec2 const& b );
/// Get rotate back(inversed) normalized iBasis
Vec2 GetInversedOrthonormaliBasis( Vec2 const& iBasisNormal );
/// Normalize degrees to -180 - 180
float NormalizeDegrees180( float degrees );

//---------------------------------------
// Products

/// Calculate the dot product of two vectors 
float DotProduct2D( Vec2 const& a, Vec2 const& b );
/// Calculate the dot product of two vectors 
float DotProduct3D( Vec3 const& a, Vec3 const& b );
/// Calculate the dot product of two vectors 
float DotProduct4D( Vec4 const& a, Vec4 const& b );
/// Calculate the cross product of two vectors
float CrossProduct2D( Vec2 const& a, Vec2 const& b );
/// Calculate the cross product of two vectors
Vec3 CrossProduct3D( Vec3 const& a, Vec3 const& b );

//---------------------------------------
// Basic 2D & 3D utilities
// Distance & projections utilities

/// Get distance between two 2D position
float GetDistance2D( Vec2 const& positionA, Vec2 const& positionB ); // -
/// Get squared distance between two 2D position - No Squared Root
float GetDistanceSquared2D( Vec2 const& positionA, Vec2 const& positionB ); // -
/// Get Manhattan distance between two 2D points
float GetTaxicabDistance2D( Vec2 const& positionA, Vec2 const& positionB );
/// Get distance between two 3D position
float GetDistance3D( Vec3 const& positionA, Vec3 const& positionB ); // -
/// Get squared distance between two 3D position - No Squared Root
float GetDistanceSquared3D( Vec3 const& positionA, Vec3 const& positionB ); // -
/// Get distance on XY between two 3D position
float GetDistanceXY3D( Vec3 const& positionA, Vec3 const& positionB ); // -
/// Get squared distance on XY between two 3D position - No Squared Root
float GetDistanceXYSquared3D( Vec3 const& positionA, Vec3 const& positionB ); // -
/// Get Manhattan distance between two int coordinate points
int GetTaxicabDistance2D( IntVec2 const& pointA, IntVec2 const& pointB );
/// Get the length of projection from one vector to another
float GetProjectedLength2D( Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto );
/// Get the vector of projection from one vector to another
Vec2 GetProjectedOnto2D( Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto );
/// Get a random point on unit circle2D
Vec2 GetRandomPointOnUnitCircle2D();
/// Get a random point in unit disc2D
Vec2 GetRandomPointInUnitDisc2D();
/// Get a random point in disc2D
Vec2 GetRandomPointInDisc2D( float discRadius, Vec2 const& discCenter = Vec2() );
/// Get a random point on unit sphere3D
Vec3 GetRandomDirection3D();
/// Get a random point in AABB2D
Vec2 GetRandomPointInAABB2D( AABB2 const& box );
/// Get a random point in AABB3D
Vec3 GetRandomPointInAABB3D( AABB3 const& box );
/// Get a random direction in cone range
Vec3 GetRandomDirectionInCone3D( Vec3 const& coneForward, float maxHalfYaw, float maxHalfPitch );

//------------------------------------
// Geometric query utilities
// 2D

/// Check whether two discs overlap with each other
bool DoDiscsOverlap( Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB ); // -
/// Check whether two AABB2 overlap with each other
bool DoAABB2sOverlap2D( AABB2 const& first, AABB2 const& second );
/// Check whether a disc and an AABB2D overlap with each other
bool DoDiscAndAABB2Overlap2D( Vec2 const& discCenter, float radius, AABB2 const& aabb2 );
/// Check whether a disc and a sector2D overlap with each other
bool DoDiscOverlapDirectedSector2D( Vec2 const& discCenter, float discRadius, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius );
/// Check whether a disc and a sector2D overlap with each other
bool DoDiscAndSectorOverlap2D( Vec2 const& discCenter, float discRadius, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius );
/// Check whether a disc and a OBB2D overlap with each other
bool DoDiscAndOBB2Overlap2D( Vec2 const& discCenter, float radius, OBB2 const& obb2 );
/// Check whether a disc and a Capsule overlap with each other
bool DoDiscAndCapsuleOverlap2D( Vec2 const& discCenter, float discRadius, Vec2 const& boneStartPoint, Vec2 const& boneEndPoint, float capsuleRadius );
/// Check whether an AABB2 and a convex hull overlap with each other
bool DoAABB2AndConvexHullOverlap2D( AABB2 const& aabb2, ConvexHull2 const& convexHull );
/// Get the distance from a point to a line segment 2D
float GetPointDistanceToLineSegment2D( Vec2 const& referencePosition, Vec2 const& startPoint, Vec2 const& endPoint );
/// Get the squared distance from a point to a line segment 2D
float GetPointDistanceToLineSegmentSquared2D( Vec2 const& referencePosition, Vec2 const& startPoint, Vec2 const& endPoint );
/// Get the distance from a point to a infinite line 2D
float GetPointDistanceToInfiniteLine2D( Vec2 const& referencePosition, Vec2 const& startPoint, Vec2 const& directionNormal );
/// Get the squared distance from a point to a infinite line 2D
float GetPointDistanceToInfiniteLineSquared2D( Vec2 const& referencePosition, Vec2 const& startPoint, Vec2 const& directionNormal );
/// Check whether a point is inside a disc
bool IsPointInsideDisc2D( Vec2 const& point, Vec2 const& discCenter, float discRadius );
/// Check whether a point is inside a sector
bool IsPointInsideOrientedSector2D( Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius );
/// Check whether a point is inside a sector
bool IsPointInsideDirectedSector2D( Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius );
/// Check whether a point is inside an AABB2D
bool IsPointInsideAABB2D( Vec2 const& point, AABB2 const& aabbBox );
/// Check whether a point is inside a capsule
bool IsPointInsideCapsule2D( Vec2 const& point, Vec2 const& boneStartPoint, Vec2 const& boneEndPoint, float radius );
/// Check whether a point is inside an OBB2D
bool IsPointInsideOBB2D( Vec2 const& point, OBB2 const& obbBox );
/// Check whether a point is inside an ConvexHull2D
bool IsPointInsideConvexHull2D( Vec2 const& point, ConvexHull2 const& convexHull );
/// Check whether a point is inside an ConvexPolygon2D
bool IsPointInsideConvexPoly2D( Vec2 const& point, ConvexPoly2 const& convexPoly );
/// Check whether a point is inside an Triangle
bool IsPointInsideTriangle2D( Vec2 const& point, Vec2 const& triVert1, Vec2 const& triVert2, Vec2 const& triVert3 );
/// Get the intersection pos of two 2D planes
Vec2 GetPlaneIntersection2D( Plane2 const& plane1, Plane2 const& plane2 );
/// Get the barycentric representation of the point
Vec3 GetBarycentricCoordinate( Vec2 const& point, Vec2 const& triVert1, Vec2 const& triVert2, Vec2 const& triVert3 );

/// Find the nearest point on a disc for a point
Vec2 GetNearestPointOnDisc2D( Vec2 const& referencePosition, Vec2 const& discCenter, float discRadius );
/// Find the nearest point on a circle(ring) for a point
Vec2 GetNearestPointOnCircle2D( Vec2 const& referencePosition, Vec2 const& circleCenter, float circleRadius );
/// Find the nearest point on an AABB2D for a point
Vec2 GetNearestPointOnAABB2D( Vec2 const& referencePosition, AABB2 const& aabbBox );
/// Find the nearest point on an infinite line for a point
Vec2 GetNearestPointOnInfiniteLine2D( Vec2 const& referencePosition, Vec2 const& point1, Vec2 const& point2 );
/// Find the nearest point on a line segment for a point
Vec2 GetNearestPointOnLineSegment2D( Vec2 const& referencePosition, Vec2 const& startPoint, Vec2 const& endPoint );
/// Find the nearest point on a capsule for a point
Vec2 GetNearestPointOnCapsule2D( Vec2 const& referencePosition, Vec2 const& boneStartPoint, Vec2 const& boneEndPoint, float radius );
/// Find the nearest point on an OBB2D for a point
Vec2 GetNearestPointOnOBB2D( Vec2 const& referencePosition, OBB2 const& obbBox );
/// Find the nearest point on a sector for a point
Vec2 GetNearestPointOnOrientedSector2D( Vec2 const& referencePosition, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius );
/// Find the nearest point on a sector for a point
Vec2 GetNearestPointOnDirectedSector2D( Vec2 const& referencePosition, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius );
/// Push a disc out of a fixed point 
// If center is the same, do nothing
bool PushDiscOutOfFixedPoint2D( Vec2& mobileDiscCenter, float discRadius, Vec2 const& fixedPoint );
/// Push a disc out of a fixed disc
// If center is the same, do nothing
bool PushDiscOutOfFixedDisc2D( Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedDiscCenter, float fixedDiscRadius );
/// Push two discs out of each other (same distance)
// If center is the same, do nothing
bool PushDiscsOutOfEachOther2D( Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius );
/// Push a disc out of a fixed AABB2D
// If center is the same, do nothing
bool PushDiscOutOfFixedAABB2D( Vec2& mobileDiscCenter, float discRadius, AABB2 const& fixedBox );
///  problem: a small disc with a great speed will tunnel
bool BounceDiscOutOfEachOther2D( Vec2& aCenter, float aRadius, Vec2& aVelocity, float aElasticity, Vec2& bCenter, float bRadius, Vec2& bVelocity, float bElasticity );
///  problem: a small disc with a great speed will tunnel
bool BounceDiscOutOfFixedPoint2D( Vec2& discCenter, float discRadius, Vec2& discVelocity, float discElasticity, Vec2 const& fixedPoint, float pointElasticity );
///  problem: a small disc with a great speed will tunnel
bool BounceDiscOutOfFixedDisc2D( Vec2& mobileCenter, float mobileRadius, Vec2& mobileVelocity, float mobileElasticity, Vec2 const& fixedCenter, float fixedRadius, float fixedElasticity );
///  problem: a small disc with a great speed will tunnel
bool BounceDiscOutOfFixedOBB2D( Vec2& mobileCenter, float mobileRadius, Vec2& mobileVelocity, float mobileElasticity, OBB2 const& fixedBox, float fixedElasticity );
///  problem: a small disc with a great speed will tunnel
bool BounceDiscOutOfFixedCapsule2D( Vec2& mobileCenter, float mobileRadius, Vec2& mobileVelocity, float mobileElasticity, Vec2 const& fixedBoneStart, Vec2 const& fixedBoneEnd, float fixedRadius, float fixedElasticity );


//------------------------------------
// 3D

///
bool IsPointInsideSphere3D( Vec3 const& point, Vec3 const& center, float radius );
///
bool IsPointInsideAABB3D( Vec3 const& point, AABB3 const& aabb3 );
///
bool IsPointInsideOBB3D( Vec3 const& point, OBB3 const& obb3 );
/// Check if AABB3s overlap with each other
bool DoAABB3sOverlap3D( AABB3 const& first, AABB3 const& second );
/// Check whether two spheres overlap with each other
bool DoSpheresOverlap( Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB );
/// check if two z-Cylinders overlap with each other
bool DoZCylindersOverlap3D( Vec2 const& cylinder1CenterXY, float cylinder1Radius, float cylinder1minZ, float cylinder1maxZ, Vec2 const& cylinder2CenterXY, float cylinder2Radius, float cylinder2minZ, float cylinder2maxZ );
///
bool DoPlanesOverlap3D( Plane3 const& plane1, Plane3 const& plane2 );
///
bool DoOBB3sOverlap3D( OBB3 const& obb1, OBB3 const& obb2 );
///
bool DoAABB3AndOBB3Overlap3D( AABB3 const& aabb, OBB3 const& obb );
///
bool DoSphereAndOBB3Overlap3D( Vec3 const& sphereCenter, float sphereRadius, OBB3 const& obb );
///
bool DoSphereAndAABB3Overlap3D( Vec3 const& sphereCenter, float sphereRadius, AABB3 const& aabb3 );
///
bool DoAABB3AndZCylinderOverlap3D( AABB3 const& aabb3, Vec2 const& cylinderCenterXY, float cylinderRadius, float cylinderminZ, float cylindermaxZ );
///
bool DoZCylinderAndSphereOverlap3D( Vec2 const& cylinderCenterXY, float cylinderRadius, float cylinderminZ, float cylindermaxZ, Vec3 const& sphereCenter, float sphereRadius );
/// 
bool DoSphereAndPlaneOverlap3D( Vec3 const& sphereCenter, float sphereRadius, Plane3 const& plane );
/// 
bool DoAABB3AndPlaneOverlap3D( AABB3 const& aabb3, Plane3 const& plane );
/// 
bool DoOBB3AndPlaneOverlap3D( OBB3 const& obb3, Plane3 const& plane );
/// 
bool DoZCylinderAndPlaneOverlap3D( Vec2 const& cylinderCenterXY, float cylinderRadius, float cylinderminZ, float cylindermaxZ, Plane3 const& plane );
///
Vec3 GetNearestPointOnAABB3D( Vec3 const& referencePosition, AABB3 const& aabb3 );
///
Vec3 GetNearestPointOnSphere3D( Vec3 const& referencePosition, Vec3 const& sphereCenter, float sphereRadius );
///
Vec3 GetNearestPointOnZCylinder3D( Vec3 const& referencePosition, Vec2 const& cylinderCenterXY, float cylinderRadius, float cylinderminZ, float cylindermaxZ );
///
Vec3 GetNearestPointOnPlane3D( Vec3 const& referencePosition, Plane3 const& plane );
///
Vec3 GetNearestPointOnOBB3D( Vec3 const& referencePosition, OBB3 const& obb3 );

/// Find the nearest point on a line segment for a point
Vec3 GetNearestPointOnLineSegment3D( Vec3 const& referencePosition, Vec3 const& startPoint, Vec3 const& endPoint );
/// Get the distance from a point to a line segment 3D
float GetPointDistanceToLineSegment3D( Vec3 const& referencePosition, Vec3 const& startPoint, Vec3 const& endPoint );
/// Get the squared distance from a point to a line segment 3D
float GetPointDistanceToLineSegmentSquared3D( Vec3 const& referencePosition, Vec3 const& startPoint, Vec3 const& endPoint );
//----------------------------
// 2D curve

///
float ComputeCubicBezier1D( float A, float B, float C, float D, float t );
/// 
float ComputeQuinticBezier1D( float A, float B, float C, float D, float E, float F, float t );
///
float SmoothStart2( float t );
///
float SmoothStart3( float t );
///
float SmoothStart4( float t );
///
float SmoothStart5( float t );
///
float SmoothStart6( float t );
///
float SmoothStop2( float t );
///
float SmoothStop3( float t );
///
float SmoothStop4( float t );
///
float SmoothStop5( float t );
///
float SmoothStop6( float t );
///
float SmoothStep3( float t );
///
float SmoothStep5( float t );
///
float Hesitate3( float t );
///
float Hesitate5( float t );

//----------------------------
// transform utilities

/// Transform a 2D position by scale, rotation and translation
void TransformPosition2D( Vec2& posToTransform, float uniformScale, float rotationDegrees, Vec2 const& translation ); // -
/// Transform a 2D position by new i, j Basis and translation
void TransformPosition2D( Vec2& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation );
/// Transform a 3D position in XY by scale, rotation and translation
void TransformPositionXY3D( Vec3& posToTransform, float uniformScale, float rotationDegrees, Vec2 const& translation ); // -
/// Transform a 3D position in XY by new i, j Basis and translation
void TransformPositionXY3D( Vec3& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation );

//----------------------------
//

/// Get the transform matrix for billboard which is always facing the camera
Mat44 const GetBillboardMatrix( BillboardType billboardType, Mat44 const& cameraMatrix, Vec3 const& billboardPosition, Vec2 const& billboardScale = Vec2( 1.f, 1.f ) );

/// Rotate a vector around an axis for certain degrees
Vec3 RotateVectorAroundAxis3D( Vec3 const& vecToTotate, Vec3 const& axisNormal, float rotationDegrees );


/// int power
/// ref: https://stackoverflow.com/questions/1505675/power-of-an-integer-in-c
int IntPow( int x, unsigned int p );