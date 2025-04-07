#include "Engine/Math/RayCastUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Plane.hpp"
#include "Engine/Math/ConvexHull2.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <cmath>
#include <float.h>

Ray2D::Ray2D( Vec2 const& startPos, Vec2 const& forwardNormal, float maxDist )
	:m_startPos(startPos)
	,m_forwardNormal(forwardNormal)
	,m_maxDist(maxDist)
{

}


Ray2D::Ray2D( Vec2 const& startPos, Vec2 const& endPos )
	:m_startPos(startPos)
{
	Vec2 forwardVector = endPos - startPos;
	float length = forwardVector.GetLength();
	m_forwardNormal = forwardVector / length;
	m_maxDist = length;
}

Ray2D::Ray2D( Vec2 const& startPos, float orientationDegrees, float maxDist )
	:m_startPos(startPos)
	,m_maxDist(maxDist)
	,m_forwardNormal(Vec2::MakeFromPolarDegrees(orientationDegrees))
{

}

Ray3D::Ray3D( Vec3 const& startPos, Vec3 const& forwardNormal, float maxDist )
	:m_startPos(startPos)
	,m_forwardNormal(forwardNormal)
	,m_maxDist(maxDist)
{

}

Ray3D::Ray3D( Vec3 const& startPos, Vec3 const& endPos )
	:m_startPos(startPos)
{
	Vec3 forwardVector = endPos - startPos;
	m_maxDist = forwardVector.GetLength();
	m_forwardNormal = forwardVector / m_maxDist;
}

bool RayCastVsDisc2D( RayCastResult2D& out_rayCastRes, Ray2D const& ray2D, Vec2 const& discCenter, float discRadius )
{
	return RayCastVsDisc2D( out_rayCastRes, ray2D.m_startPos, ray2D.m_forwardNormal, ray2D.m_maxDist, discCenter, discRadius );
}

bool RayCastVsDisc2D( RayCastResult2D& out_rayCastRes, Vec2 const& startPos, Vec2 const& forwardNormal, float maxDist, Vec2 const& discCenter, float discRadius )
{
	Vec2 const& iBasisNormal = forwardNormal;
	Vec2 jBasisNormal = Vec2( -forwardNormal.y, forwardNormal.x );
	Vec2 SC = discCenter - startPos;
	float SCj = DotProduct2D( SC, jBasisNormal );
	// the infinite ray cannot hit disc out of radius
	if (SCj >= discRadius || SCj <= -discRadius) {
		return false;
	}
	float a = sqrtf( discRadius * discRadius - SCj * SCj );
	float SCi = DotProduct2D( SC, iBasisNormal );
	float impactLength = SCi - a;
	// start point inside disc, hit when start
	if (impactLength > -2 * a && impactLength < 0) {
		out_rayCastRes.m_didImpact = true;
		out_rayCastRes.m_impactDist = 0.f;
		out_rayCastRes.m_impactNormal = -forwardNormal;
		out_rayCastRes.m_impactPos = startPos;
		out_rayCastRes.m_rayForwardNormal = forwardNormal;
		out_rayCastRes.m_rayMaxLength = maxDist;
		out_rayCastRes.m_rayStartPos = startPos;
		return true;
	}
	// length is short or direction is reversed, cannot get to disc
	if (impactLength <= 0.f || impactLength >= maxDist) {
		return false;
	}
	// hit the disc
	Vec2 impactPos = impactLength * iBasisNormal + startPos;
	out_rayCastRes.m_didImpact = true;
	out_rayCastRes.m_impactDist = impactLength;
	out_rayCastRes.m_impactNormal = (impactPos - discCenter) / discRadius;
	out_rayCastRes.m_impactPos = impactPos;
	out_rayCastRes.m_rayForwardNormal = forwardNormal;
	out_rayCastRes.m_rayMaxLength = maxDist;
	out_rayCastRes.m_rayStartPos = startPos;
	return true;
}

bool RayCastVsDisc2D( RayCastResult2D& out_rayCastRes, Vec2 const& startPos, float forwardDegrees, float maxDist, Vec2 const& discCenter, float discRadius )
{
	return RayCastVsDisc2D( out_rayCastRes, startPos, Vec2::MakeFromPolarDegrees( forwardDegrees ), maxDist, discCenter, discRadius );
}

bool RayCastVsDisc2D( RayCastResult2D& out_rayCastRes, Vec2 const& startPos, Vec2 const& endPos, Vec2 const& discCenter, float discRadius )
{
	float rayLength = (endPos - startPos).GetLength();
	return RayCastVsDisc2D( out_rayCastRes, startPos, (endPos - startPos) / rayLength, rayLength, discCenter, discRadius );
}

bool RayCastVsLineSegment2D( RayCastResult2D& out_rayCastRes, Vec2 const& startPos, Vec2 const& forwardNormal, float maxDist, Vec2 const& lineStartPos, Vec2 const& lineEndPos )
{
	Vec2 leftNormal = Vec2( forwardNormal.y, -forwardNormal.x );
	Vec2 SS = lineStartPos - startPos;
	Vec2 SE = lineEndPos - startPos;
	float SSOnLeft = DotProduct2D( SS, leftNormal );
	float SEOnLeft = DotProduct2D( SE, leftNormal );
	if ((SSOnLeft >= 0.f && SEOnLeft >= 0.f) || (SSOnLeft <= 0.f && SEOnLeft <= 0.f)) {
		return false;
	}
	float t = SSOnLeft / (SSOnLeft - SEOnLeft);
	Vec2 lineSegForward = lineEndPos - lineStartPos;
	Vec2 impactPos = lineStartPos + t * lineSegForward;
	Vec2 SI = impactPos - startPos;
	float impactLength = DotProduct2D( SI, forwardNormal );
	if (impactLength <= 0.f || impactLength >= maxDist) {
		return false;
	}
	// hit
	lineSegForward.Normalize();
	out_rayCastRes.m_impactNormal = Vec2( lineSegForward.y, -lineSegForward.x );
	if (SSOnLeft > 0.f) {
		out_rayCastRes.m_impactNormal = -out_rayCastRes.m_impactNormal;
	}
	out_rayCastRes.m_didImpact = true;
	out_rayCastRes.m_impactDist = impactLength;
	out_rayCastRes.m_impactPos = impactPos;
	out_rayCastRes.m_rayForwardNormal = forwardNormal;
	out_rayCastRes.m_rayMaxLength = maxDist;
	out_rayCastRes.m_rayStartPos = startPos;
	return true;
}

bool RayCastVsAABB2D( RayCastResult2D& out_rayCastRes, Vec2 const& startPos, Vec2 const& forwardNormal, float maxDist, AABB2 const& aabb2 )
{
	Vec2 endPos = startPos + forwardNormal * maxDist;
	if (!DoAABB2sOverlap2D( AABB2( Vec2( Minf( startPos.x, endPos.x ), Minf( startPos.y, endPos.y ) ), Vec2( Maxf( startPos.x, endPos.x ), Maxf( startPos.y, endPos.y ) ) ), aabb2 )) {
		out_rayCastRes.m_didImpact = false;
		return false;
	}
	else if (aabb2.IsPointInside( startPos )) {
		// ray from inside AABB2
		out_rayCastRes.m_didImpact = true;
		out_rayCastRes.m_impactDist = 0.f;
		out_rayCastRes.m_impactNormal = -forwardNormal;
		out_rayCastRes.m_impactPos = startPos;
		out_rayCastRes.m_rayForwardNormal = forwardNormal;
		out_rayCastRes.m_rayMaxLength = maxDist;
		out_rayCastRes.m_rayStartPos = startPos;
		return true;
	}
	else if (forwardNormal.x == 0) {
		if (startPos.x <= aabb2.m_mins.x || startPos.x >= aabb2.m_maxs.x) {
			// out side, not hit
			out_rayCastRes.m_didImpact = false;
			return false;
		}
		float oneOverRangeY = 1.f / (endPos.y - startPos.y);
		float minYHitT = (aabb2.m_mins.y - startPos.y) * oneOverRangeY;
		float maxYHitT = (aabb2.m_maxs.y - startPos.y) * oneOverRangeY;
		if (minYHitT < maxYHitT && minYHitT >= 0.f) {
			// hit bottom
			out_rayCastRes.m_didImpact = true;
			out_rayCastRes.m_impactDist = maxDist * minYHitT;
			out_rayCastRes.m_impactNormal = Vec2( 0.f, -1.f );
			out_rayCastRes.m_impactPos = startPos + forwardNormal * out_rayCastRes.m_impactDist;
			out_rayCastRes.m_rayForwardNormal = forwardNormal;
			out_rayCastRes.m_rayMaxLength = maxDist;
			out_rayCastRes.m_rayStartPos = startPos;
			return true;
		}
		else if(minYHitT >= maxYHitT && maxYHitT >= 0.f) {
			// hit top
			out_rayCastRes.m_didImpact = true;
			out_rayCastRes.m_impactDist = maxDist * maxYHitT;
			out_rayCastRes.m_impactNormal = Vec2( 0.f, 1.f );
			out_rayCastRes.m_impactPos = startPos + forwardNormal * out_rayCastRes.m_impactDist;
			out_rayCastRes.m_rayForwardNormal = forwardNormal;
			out_rayCastRes.m_rayMaxLength = maxDist;
			out_rayCastRes.m_rayStartPos = startPos;
			return true;
		}
		else {
			// not hit
			out_rayCastRes.m_didImpact = false;
			return false;
		}
	}
	else if (forwardNormal.y == 0) {
		if (startPos.y <= aabb2.m_mins.y || startPos.y >= aabb2.m_maxs.y) {
			// out side, not hit
			out_rayCastRes.m_didImpact = false;
			return false;
		}
		float oneOverRangeX = 1.f / (endPos.x - startPos.x);
		float minXHitT = (aabb2.m_mins.x - startPos.x) * oneOverRangeX;
		float maxXHitT = (aabb2.m_maxs.x - startPos.x) * oneOverRangeX;
		if (minXHitT < maxXHitT && minXHitT >= 0.f) {
			// hit left
			out_rayCastRes.m_didImpact = true;
			out_rayCastRes.m_impactDist = maxDist * minXHitT;
			out_rayCastRes.m_impactNormal = Vec2( -1.f, 0.f );
			out_rayCastRes.m_impactPos = startPos + forwardNormal * out_rayCastRes.m_impactDist;
			out_rayCastRes.m_rayForwardNormal = forwardNormal;
			out_rayCastRes.m_rayMaxLength = maxDist;
			out_rayCastRes.m_rayStartPos = startPos;
			return true;
		}
		else if (maxXHitT <= minXHitT && maxXHitT >= 0.f) {
			// hit right
			out_rayCastRes.m_didImpact = true;
			out_rayCastRes.m_impactDist = maxDist * maxXHitT;
			out_rayCastRes.m_impactNormal = Vec2( 1.f, 0.f );
			out_rayCastRes.m_impactPos = startPos + forwardNormal * out_rayCastRes.m_impactDist;
			out_rayCastRes.m_rayForwardNormal = forwardNormal;
			out_rayCastRes.m_rayMaxLength = maxDist;
			out_rayCastRes.m_rayStartPos = startPos;
			return true;
		}
		else {
			// not hit
			out_rayCastRes.m_didImpact = false;
			return false;
		}
	}
	else {
		float oneOverRangeX = 1.f / (endPos.x - startPos.x);
		float minXHitT = (aabb2.m_mins.x - startPos.x) * oneOverRangeX;
		float maxXHitT = (aabb2.m_maxs.x - startPos.x) * oneOverRangeX;
		float firstXHitT, secondXHitT;
		if (minXHitT < maxXHitT) {
			firstXHitT = minXHitT;
			secondXHitT = maxXHitT;
		}
		else {
			firstXHitT = maxXHitT;
			secondXHitT = minXHitT;
		}

		float oneOverRangeY = 1.f / (endPos.y - startPos.y);
		float minYHitT = (aabb2.m_mins.y - startPos.y) * oneOverRangeY;
		float maxYHitT = (aabb2.m_maxs.y - startPos.y) * oneOverRangeY;
		float firstYHitT, secondYHitT;
		if (minYHitT < maxYHitT) {
			firstYHitT = minYHitT;
			secondYHitT = maxYHitT;
		}
		else {
			firstYHitT = maxYHitT;
			secondYHitT = minYHitT;
		}

		if (firstXHitT < firstYHitT) {
			if (firstYHitT >= secondXHitT) {
				// not hit
				out_rayCastRes.m_didImpact = false;
				return false;
			}
			else {
				if (firstYHitT <= 0.f || firstYHitT >= 1.f) {
					// out of range
					out_rayCastRes.m_didImpact = false;
					return false;
				}
				// hit
				else if (firstYHitT == minYHitT) {
					// hit bottom
					out_rayCastRes.m_didImpact = true;
					out_rayCastRes.m_impactDist = maxDist * firstYHitT;
					out_rayCastRes.m_impactNormal = Vec2( 0.f, -1.f );
					out_rayCastRes.m_impactPos = startPos + forwardNormal * out_rayCastRes.m_impactDist;
					out_rayCastRes.m_rayForwardNormal = forwardNormal;
					out_rayCastRes.m_rayMaxLength = maxDist;
					out_rayCastRes.m_rayStartPos = startPos;
					return true;
				}
				else {
					// hit top
					out_rayCastRes.m_didImpact = true;
					out_rayCastRes.m_impactDist = maxDist * firstYHitT;
					out_rayCastRes.m_impactNormal = Vec2( 0.f, 1.f );
					out_rayCastRes.m_impactPos = startPos + forwardNormal * out_rayCastRes.m_impactDist;
					out_rayCastRes.m_rayForwardNormal = forwardNormal;
					out_rayCastRes.m_rayMaxLength = maxDist;
					out_rayCastRes.m_rayStartPos = startPos;
					return true;
				}
			}
		}
		else/*firstYHitT <= firstXHitT*/ {
			if (firstXHitT >= secondYHitT) {
				// not hit
				out_rayCastRes.m_didImpact = false;
				return false;
			}
			else {
				if (firstXHitT <= 0.f || firstXHitT >= 1.f) {
					// out of range
					out_rayCastRes.m_didImpact = false;
					return false;
				}
				// hit
				if (firstXHitT == minXHitT) {
					// hit left
					out_rayCastRes.m_didImpact = true;
					out_rayCastRes.m_impactDist = maxDist * firstXHitT;
					out_rayCastRes.m_impactNormal = Vec2( -1.f, 0.f );
					out_rayCastRes.m_impactPos = startPos + forwardNormal * out_rayCastRes.m_impactDist;
					out_rayCastRes.m_rayForwardNormal = forwardNormal;
					out_rayCastRes.m_rayMaxLength = maxDist;
					out_rayCastRes.m_rayStartPos = startPos;
					return true;
				}
				else {
					// hit right
					out_rayCastRes.m_didImpact = true;
					out_rayCastRes.m_impactDist = maxDist * firstXHitT;
					out_rayCastRes.m_impactNormal = Vec2( 1.f, 0.f );
					out_rayCastRes.m_impactPos = startPos + forwardNormal * out_rayCastRes.m_impactDist;
					out_rayCastRes.m_rayForwardNormal = forwardNormal;
					out_rayCastRes.m_rayMaxLength = maxDist;
					out_rayCastRes.m_rayStartPos = startPos;
					return true;
				}
			}
		}
	}
}

bool RayCastVsAABB2DResultOnly( Vec2 const& startPos, Vec2 const& forwardNormal, float maxDist, AABB2 const& aabb2 )
{
	Vec2 endPos = startPos + forwardNormal * maxDist;
	if (!DoAABB2sOverlap2D( AABB2( Vec2( Minf( startPos.x, endPos.x ), Minf( startPos.y, endPos.y ) ), Vec2( Maxf( startPos.x, endPos.x ), Maxf( startPos.y, endPos.y ) ) ), aabb2 )) {

		return false;
	}
	else if (aabb2.IsPointInside( startPos )) {
		return true;
	}

	else if (forwardNormal.x == 0) {
		if (startPos.x <= aabb2.m_mins.x || startPos.x >= aabb2.m_maxs.x) {
			// out side, not hit
			return false;
		}
		float oneOverRangeY = 1.f / (endPos.y - startPos.y);
		float minYHitT = (aabb2.m_mins.y - startPos.y) * oneOverRangeY;
		float maxYHitT = (aabb2.m_maxs.y - startPos.y) * oneOverRangeY;
		if (minYHitT < maxYHitT && minYHitT >= 0.f) {
			// hit bottom
			return true;
		}
		else if (minYHitT >= maxYHitT && maxYHitT >= 0.f) {
			// hit top
			return true;
		}
		else {
			// not hit
			return false;
		}
	}
	else if (forwardNormal.y == 0) {
		if (startPos.y <= aabb2.m_mins.y || startPos.y >= aabb2.m_maxs.y) {
			// out side, not hit
			return false;
		}
		float oneOverRangeX = 1.f / (endPos.x - startPos.x);
		float minXHitT = (aabb2.m_mins.x - startPos.x) * oneOverRangeX;
		float maxXHitT = (aabb2.m_maxs.x - startPos.x) * oneOverRangeX;
		if (minXHitT < maxXHitT && minXHitT >= 0.f) {
			// hit left
			return true;
		}
		else if (maxXHitT <= minXHitT && maxXHitT >= 0.f) {
			// hit right
			return true;
		}
		else {
			// not hit
			return false;
		}
	}
	else {
		float oneOverRangeX = 1.f / (endPos.x - startPos.x);
		float minXHitT = (aabb2.m_mins.x - startPos.x) * oneOverRangeX;
		float maxXHitT = (aabb2.m_maxs.x - startPos.x) * oneOverRangeX;
		float firstXHitT, secondXHitT;
		if (minXHitT < maxXHitT) {
			firstXHitT = minXHitT;
			secondXHitT = maxXHitT;
		}
		else {
			firstXHitT = maxXHitT;
			secondXHitT = minXHitT;
		}

		float oneOverRangeY = 1.f / (endPos.y - startPos.y);
		float minYHitT = (aabb2.m_mins.y - startPos.y) * oneOverRangeY;
		float maxYHitT = (aabb2.m_maxs.y - startPos.y) * oneOverRangeY;
		float firstYHitT, secondYHitT;
		if (minYHitT < maxYHitT) {
			firstYHitT = minYHitT;
			secondYHitT = maxYHitT;
		}
		else {
			firstYHitT = maxYHitT;
			secondYHitT = minYHitT;
		}

		if (firstXHitT < firstYHitT) {
			if (firstYHitT >= secondXHitT) {
				// not hit
				return false;
			}
			else {
				if (firstYHitT <= 0.f || firstYHitT >= 1.f) {
					// out of range
					return false;
				}
				return true;	
			}
		}
		else/*firstYHitT <= firstXHitT*/ {
			if (firstXHitT >= secondYHitT) {
				// not hit
				return false;
			}
			else {
				if (firstXHitT <= 0.f || firstXHitT >= 1.f) {
					// out of range
					return false;
				}
				// hit
				return true;
			}
		}
	}
}

bool RayCastVsConvexHull2D( RayCastResult2D& out_rayCastRes, Vec2 const& startPos, Vec2 const& forwardNormal, float maxDist, ConvexHull2 const& convexHull )
{
	// check the ray start point and all plane2D
	bool hasOutsidePlane = false;
	float maxTravelDist = -1.f;
	int maxDistPlaneIndex = -1;
	for (int i = 0; i < (int)convexHull.m_boundingPlanes.size(); ++i) {
		Plane2 const& plane = convexHull.m_boundingPlanes[i];
		float startPosAltitude = plane.GetAltitudeOfPoint( startPos );
		if (startPosAltitude > 0.f) { // outside a plane
			hasOutsidePlane = true;
			float NdotF = DotProduct2D( forwardNormal, plane.m_normal );
			if (NdotF < 0.f) { // ray forward direction is going to the plane
				float dist = 0.f;
				float SdotN = DotProduct2D( startPos, plane.m_normal );
				dist = (plane.m_distanceFromOrigin - SdotN) / NdotF; // calculate the impact distance (ray vs. plane)
				if (dist > 0.f && dist < maxDist && dist > maxTravelDist) { // check the max distance requirement
					maxTravelDist = dist; // find the latest enter point
					maxDistPlaneIndex = i;
				}
			}
		}
		else if (startPosAltitude == 0.f) { // on a plane/ may start on the edges of the polygon
			float NdotF = DotProduct2D( forwardNormal, plane.m_normal );
			if (NdotF < 0.f) { // ray starts from one edge and goes into the shape
				bool onShape = true;
				for (int j = 0; j < (int)convexHull.m_boundingPlanes.size(); ++j) {
					if (i != j && convexHull.m_boundingPlanes[j].GetAltitudeOfPoint( startPos ) > 0.001f) {
						onShape = false;
						break;
					}
				}
				if (onShape) {
					out_rayCastRes.m_didImpact = true;
					out_rayCastRes.m_impactDist = 0.f;
					out_rayCastRes.m_impactNormal = -forwardNormal;
					out_rayCastRes.m_impactPos = startPos;
					out_rayCastRes.m_rayForwardNormal = forwardNormal;
					out_rayCastRes.m_rayMaxLength = maxDist;
					out_rayCastRes.m_rayStartPos = startPos;
					return true;
				}
			}
		}
	}

	if (maxTravelDist > 0.f) {
		// check if the last enter point is on the convex hull
		Vec2 impactPos = startPos + maxTravelDist * forwardNormal;
		bool onShape = true;
		for (int i = 0; i < (int)convexHull.m_boundingPlanes.size(); ++i) {
			if (i != maxDistPlaneIndex && convexHull.m_boundingPlanes[i].GetAltitudeOfPoint( impactPos ) > 0.001f) {
				onShape = false;
				break;
			}
		}
		if (onShape) {
			out_rayCastRes.m_didImpact = true;
			out_rayCastRes.m_impactDist = maxTravelDist;
			out_rayCastRes.m_impactNormal = convexHull.m_boundingPlanes[maxDistPlaneIndex].m_normal;
			out_rayCastRes.m_impactPos = impactPos;
			out_rayCastRes.m_rayForwardNormal = forwardNormal;
			out_rayCastRes.m_rayMaxLength = maxDist;
			out_rayCastRes.m_rayStartPos = startPos;
			return true;
		}
	}

	// ray start in the convex hull, just return the start pos
	if (!hasOutsidePlane) {
		out_rayCastRes.m_didImpact = true;
		out_rayCastRes.m_impactDist = 0.f;
		out_rayCastRes.m_impactNormal = -forwardNormal;
		out_rayCastRes.m_impactPos = startPos;
		out_rayCastRes.m_rayForwardNormal = forwardNormal;
		out_rayCastRes.m_rayMaxLength = maxDist;
		out_rayCastRes.m_rayStartPos = startPos;
		return true;
	}

	out_rayCastRes.m_didImpact = false;
	return false;
}

bool RayCastVsCylinderZ3D( RayCastResult3D& out_rayCastRes, Vec3 const& startPos, Vec3 const& forwardNormal, float maxDist, Vec2 const& cylinderCenter, float minZ, float maxZ, float radius )
{
	float distanceTravel = 0.f;
	Vec3 hitPos;
	out_rayCastRes.m_didImpact = false;
	// check if hit top or bottom
	if (startPos.z >= maxZ) {
		// ray goes up do not hit
		if (forwardNormal.z < 0) {
		    distanceTravel = (maxZ - startPos.z) / forwardNormal.z;
		    hitPos = startPos + forwardNormal * distanceTravel;
			if (distanceTravel < maxDist && IsPointInsideDisc2D( hitPos, cylinderCenter, radius )) {
				// hit
				out_rayCastRes.m_didImpact = true;
				out_rayCastRes.m_impactDist = distanceTravel;
				out_rayCastRes.m_impactNormal = Vec3( 0.f, 0.f, 1.f );
				out_rayCastRes.m_impactPos = hitPos;
				out_rayCastRes.m_rayForwardNormal = forwardNormal;
				out_rayCastRes.m_rayMaxLength = maxDist;
				out_rayCastRes.m_rayStartPos = startPos;
				return true;
			}
		}
		else {
			return false;
		}
	}
	else if (startPos.z <= minZ) {
		// ray goes up do not hit
		if (forwardNormal.z > 0) {
			distanceTravel = (minZ - startPos.z) / forwardNormal.z;
			hitPos = startPos + forwardNormal * distanceTravel;
			if (distanceTravel < maxDist && IsPointInsideDisc2D( hitPos, cylinderCenter, radius )) {
				// hit
				out_rayCastRes.m_didImpact = true;
				out_rayCastRes.m_impactDist = distanceTravel;
				out_rayCastRes.m_impactNormal = Vec3( 0.f, 0.f, -1.f );
				out_rayCastRes.m_impactPos = hitPos;
				out_rayCastRes.m_rayForwardNormal = forwardNormal;
				out_rayCastRes.m_rayMaxLength = maxDist;
				out_rayCastRes.m_rayStartPos = startPos;
				return true;
			}
		}
		else {
			return false;
		}
	}
	else if(IsPointInsideDisc2D( startPos, cylinderCenter, radius )) {
		// start inside cylinder
		out_rayCastRes.m_didImpact = true;
		out_rayCastRes.m_impactDist = 0.f;
		out_rayCastRes.m_impactNormal = -forwardNormal;
		out_rayCastRes.m_impactPos = startPos;
		out_rayCastRes.m_rayForwardNormal = forwardNormal;
		out_rayCastRes.m_rayMaxLength = maxDist;
		out_rayCastRes.m_rayStartPos = startPos;
		return true;
	}
	
	// hit side
	float normalLength2D = Vec2( forwardNormal.x, forwardNormal.y ).GetLength();
	if (normalLength2D == 0.f) {
		return false;
	}
	// try to ray cast 2D disc
	RayCastResult2D res2D;
	bool isHit = RayCastVsDisc2D( res2D, Vec2( startPos ), forwardNormal / normalLength2D, maxDist * normalLength2D, cylinderCenter, radius );

	if (!isHit) {
		return false;
	}

	distanceTravel = res2D.m_impactDist / normalLength2D;
	
	// get the hit position
	hitPos = Vec3( res2D.m_impactPos.x, res2D.m_impactPos.y, startPos.z + distanceTravel * forwardNormal.z );
	// pass the cylinder without hitting
	if (hitPos.z > maxZ || hitPos.z < minZ) {
		return false;
	}
	if (out_rayCastRes.m_didImpact && out_rayCastRes.m_impactDist < distanceTravel) {
		return true;
	}
	// hit side successfully
	out_rayCastRes.m_didImpact = true;
	out_rayCastRes.m_impactDist = distanceTravel;
	out_rayCastRes.m_impactNormal = Vec3( res2D.m_impactNormal.x, res2D.m_impactNormal.y, 0.f );
	out_rayCastRes.m_impactPos = hitPos;
	out_rayCastRes.m_rayForwardNormal = forwardNormal;
	out_rayCastRes.m_rayMaxLength = maxDist;
	out_rayCastRes.m_rayStartPos = startPos;
	return true;
}

bool RayCastVsCylinderZ3D( RayCastResult3D& out_rayCastRes, Ray3D const& ray3D, Vec2 const& cylinderCenter, float minZ, float maxZ, float radius )
{
	return RayCastVsCylinderZ3D( out_rayCastRes, ray3D.m_startPos, ray3D.m_forwardNormal, ray3D.m_maxDist, cylinderCenter, minZ, maxZ, radius );
}

bool RayCastVsSphere3D( RayCastResult3D& out_rayCastRes, Vec3 const& startPos, Vec3 const& forwardNormal, float maxDist, Vec3 const& center, float radius )
{

	Vec3 SC = center - startPos;
	//Vec3 upVector = CrossProduct3D( forwardNormal, SC );
	//Vec3 leftNormal = CrossProduct3D( upVector, forwardNormal ).GetNormalized();
	float SCi = DotProduct3D( SC, forwardNormal );
	Vec3 leftVector = (SC - forwardNormal * SCi);
	/*
	if (leftVector == Vec3( 0.f, 0.f, 0.f )) {
		// ray go through the center of the sphere
		float impactLength = SC.GetLength() - radius;
		// start point inside disc, hit when start
		if (impactLength > -2 * radius && impactLength < 0) {
			out_rayCastRes.m_didImpact = true;
			out_rayCastRes.m_impactDist = 0.f;
			out_rayCastRes.m_impactNormal = -forwardNormal;
			out_rayCastRes.m_impactPos = startPos;
			out_rayCastRes.m_rayForwardNormal = forwardNormal;
			out_rayCastRes.m_rayMaxLength = maxDist;
			out_rayCastRes.m_rayStartPos = startPos;
			return true;
		}
		// length is short or direction is reversed, cannot get to disc
		if (impactLength <= 0.f || impactLength >= maxDist) {
			return false;
		}
		Vec3 impactPos = impactLength * forwardNormal + startPos;
		out_rayCastRes.m_didImpact = true;
		out_rayCastRes.m_impactDist = impactLength;
		out_rayCastRes.m_impactNormal = (impactPos - center) / radius;
		out_rayCastRes.m_impactPos = impactPos;
		out_rayCastRes.m_rayForwardNormal = forwardNormal;
		out_rayCastRes.m_rayMaxLength = maxDist;
		out_rayCastRes.m_rayStartPos = startPos;
		return true;
	}*/

	float squaredSCj = leftVector.GetLengthSquared();
	float squaredA = radius * radius - squaredSCj;
	if (squaredA <= 0.f) {
		return false;
	}

	float a = sqrtf( squaredA );
	float impactLength = SCi - a;

	// start point inside disc, hit when start
	if (impactLength > -2 * a && impactLength < 0) {
		out_rayCastRes.m_didImpact = true;
		out_rayCastRes.m_impactDist = 0.f;
		out_rayCastRes.m_impactNormal = -forwardNormal;
		out_rayCastRes.m_impactPos = startPos;
		out_rayCastRes.m_rayForwardNormal = forwardNormal;
		out_rayCastRes.m_rayMaxLength = maxDist;
		out_rayCastRes.m_rayStartPos = startPos;
		return true;
	}
	// length is short or direction is reversed, cannot get to disc
	if (impactLength <= 0.f || impactLength >= maxDist) {
		return false;
	}
	// hit the disc
	Vec3 impactPos = impactLength * forwardNormal + startPos;
	out_rayCastRes.m_didImpact = true;
	out_rayCastRes.m_impactDist = impactLength;
	out_rayCastRes.m_impactNormal = (impactPos - center) / radius;
	out_rayCastRes.m_impactPos = impactPos;
	out_rayCastRes.m_rayForwardNormal = forwardNormal;
	out_rayCastRes.m_rayMaxLength = maxDist;
	out_rayCastRes.m_rayStartPos = startPos;
	return true;
}


bool RayCastVsAABB3D( RayCastResult3D& out_rayCastRes, Vec3 const& startPos, Vec3 const& forwardNormal, float maxDist, AABB3 const& aabb3 )
{
	Vec3 endPos = startPos + forwardNormal * maxDist;
	if (!DoAABB3sOverlap3D( AABB3( Vec3( Minf( startPos.x, endPos.x ), Minf( startPos.y, endPos.y ), Minf( startPos.z, endPos.z ) ), Vec3( Maxf( startPos.x, endPos.x ), Maxf( startPos.y, endPos.y ), Maxf( startPos.z, endPos.z ) ) ), aabb3 )) {
		out_rayCastRes.m_didImpact = false;
		return false;
	}

	if (IsPointInsideAABB3D( startPos, aabb3 )) {
		out_rayCastRes.m_didImpact = true;
		out_rayCastRes.m_impactDist = 0.f;
		out_rayCastRes.m_impactNormal = -forwardNormal;
		out_rayCastRes.m_impactPos = startPos;
		out_rayCastRes.m_rayForwardNormal = forwardNormal;
		out_rayCastRes.m_rayMaxLength = maxDist;
		out_rayCastRes.m_rayStartPos = startPos;
		return true;
	}

	// 0 -x 1 +x 2 -y 3 +y 4 -z 5 +z
	int hitFace = -1;
	int hitFaceX = -1, hitFaceY = -1, hitFaceZ = -1;
	float oneOverRangeX = 1.f / (endPos.x - startPos.x);
	float minXHitT = (aabb3.m_mins.x - startPos.x) * oneOverRangeX;
	float maxXHitT = (aabb3.m_maxs.x - startPos.x) * oneOverRangeX;
	float firstXHitT, secondXHitT;
	if (forwardNormal.x == 0) {
		if (startPos.x > aabb3.m_mins.x && startPos.x < aabb3.m_maxs.x) {
			firstXHitT = -FLT_MAX;
			secondXHitT = FLT_MAX;
		}
		else {
			return false;
		}
	}
	else if (minXHitT < maxXHitT) {
		firstXHitT = minXHitT;
		secondXHitT = maxXHitT;
		hitFaceX = 0;
	}
	else {
		firstXHitT = maxXHitT;
		secondXHitT = minXHitT;
		hitFaceX = 1;
	}

	float oneOverRangeY = 1.f / (endPos.y - startPos.y);
	float minYHitT = (aabb3.m_mins.y - startPos.y) * oneOverRangeY;
	float maxYHitT = (aabb3.m_maxs.y - startPos.y) * oneOverRangeY;
	float firstYHitT, secondYHitT;

	if (forwardNormal.y == 0) {
		if (startPos.y > aabb3.m_mins.y && startPos.y < aabb3.m_maxs.y) {
			firstYHitT = -FLT_MAX;
			secondYHitT = FLT_MAX;
		}
		else {
			return false;
		}
	}
	else if (minYHitT < maxYHitT) {
		firstYHitT = minYHitT;
		secondYHitT = maxYHitT;
		hitFaceY = 2;
	}
	else {
		firstYHitT = maxYHitT;
		secondYHitT = minYHitT;
		hitFaceY = 3;
	}

	float firstXYInt, secondXYInt;
	if (secondXHitT > firstYHitT && secondYHitT > firstXHitT) {
		if (firstXHitT < firstYHitT) {
			firstXYInt = firstYHitT;
			hitFace = hitFaceY;
		}
		else {
			firstXYInt = firstXHitT;
			hitFace = hitFaceX;
		}

		if (secondXHitT < secondYHitT) {
			secondXYInt = secondXHitT;
		}
		else {
			secondXYInt = secondYHitT;
		}
	}
	else {
		out_rayCastRes.m_didImpact = false;
		return false;
	}

	float oneOverRangeZ = 1.f / (endPos.z - startPos.z);
	float minZHitT = (aabb3.m_mins.z - startPos.z) * oneOverRangeZ;
	float maxZHitT = (aabb3.m_maxs.z - startPos.z) * oneOverRangeZ;
	float firstZHitT, secondZHitT;

	if (forwardNormal.z == 0) {
		if (startPos.z > aabb3.m_mins.z && startPos.z < aabb3.m_maxs.z) {
			firstZHitT = -FLT_MAX;
			secondZHitT = FLT_MAX;
		}
		else {
			return false;
		}
	}
	else if (minZHitT < maxZHitT) {
		firstZHitT = minZHitT;
		secondZHitT = maxZHitT;
		hitFaceZ = 4;
	}
	else {
		firstZHitT = maxZHitT;
		secondZHitT = minZHitT;
		hitFaceZ = 5;
	}

	float firstXYZInt;
	if (secondXYInt > firstZHitT && secondZHitT > firstXYInt) {
		if (firstXYInt > firstZHitT) {
			firstXYZInt = firstXYInt;
		}
		else {
			firstXYZInt = firstZHitT;
			hitFace = hitFaceZ;
		}
	}
	else {
		out_rayCastRes.m_didImpact = false;
		return false;
	}
	out_rayCastRes.m_didImpact = true;
	out_rayCastRes.m_impactDist = firstXYZInt * maxDist;

	switch (hitFace)
	{
	case 0:	out_rayCastRes.m_impactNormal = Vec3( -1.f, 0.f, 0.f ); break;
	case 1:	out_rayCastRes.m_impactNormal = Vec3( 1.f, 0.f, 0.f ); break;
	case 2:	out_rayCastRes.m_impactNormal = Vec3( 0.f, -1.f, 0.f ); break;
	case 3:	out_rayCastRes.m_impactNormal = Vec3( 0.f, 1.f, 0.f ); break;
	case 4:	out_rayCastRes.m_impactNormal = Vec3( 0.f, 0.f, -1.f ); break;
	case 5:	out_rayCastRes.m_impactNormal = Vec3( 0.f, 0.f, 1.f ); break;
	}
	out_rayCastRes.m_impactPos = startPos + forwardNormal * out_rayCastRes.m_impactDist;
	out_rayCastRes.m_rayForwardNormal = forwardNormal;
	out_rayCastRes.m_rayMaxLength = maxDist;
	out_rayCastRes.m_rayStartPos = startPos;

	return true;
}

bool RayCastVsOBB3D( RayCastResult3D& out_rayCastRes, Vec3 const& startPos, Vec3 const& forwardNormal, float maxDist, OBB3 const& obb3 )
{
	Mat44 obb3Matrix( obb3.m_iBasis, obb3.m_jBasis, obb3.m_kBasis, obb3.m_center );
	Mat44 transformMatrix = obb3Matrix.GetOrthonormalInverse();
	AABB3 localAABB3( -obb3.m_halfDimensions, obb3.m_halfDimensions );
	Vec3 newStartPos = transformMatrix.TransformPosition3D( startPos );
	Vec3 newForwardNormal = transformMatrix.TransformVectorQuantity3D( forwardNormal );
	
	bool result = RayCastVsAABB3D( out_rayCastRes, newStartPos, newForwardNormal, maxDist, localAABB3 );

	if (!result) {
		return false;
	}

	out_rayCastRes.m_impactNormal = obb3Matrix.TransformVectorQuantity3D( out_rayCastRes.m_impactNormal );
	out_rayCastRes.m_impactPos = obb3Matrix.TransformPosition3D( out_rayCastRes.m_impactPos );
	out_rayCastRes.m_rayForwardNormal = forwardNormal;
	out_rayCastRes.m_rayStartPos = startPos;
	return true;
}

bool RayCastVsPlane3D( RayCastResult3D& out_rayCastRes, Vec3 const& startPos, Vec3 const& forwardNormal, float maxDist, Plane3 const& plane )
{
	Vec3 endPos = startPos + forwardNormal * maxDist;

	float startPosAltitude = plane.GetAltitudeOfPoint( startPos );
	float endPosAltitude = plane.GetAltitudeOfPoint( endPos );

	// all on one side of the plane
	if ((startPosAltitude <= 0.f && endPosAltitude <= 0.f) || (startPosAltitude >= 0.f && endPosAltitude >= 0.f)) {
		return false;
	}

	// start from the back side of the plane, so hit the back side and the normal is negative
	if (startPosAltitude < 0.f) {
		out_rayCastRes.m_impactNormal = -plane.m_normal;
	}
	else {
		out_rayCastRes.m_impactNormal = plane.m_normal;
	}

	startPosAltitude = abs( startPosAltitude );
	endPosAltitude = abs( endPosAltitude );

	out_rayCastRes.m_didImpact = true;
	out_rayCastRes.m_impactDist = maxDist * (startPosAltitude / (startPosAltitude + endPosAltitude));
	out_rayCastRes.m_impactPos = startPos + forwardNormal * out_rayCastRes.m_impactDist;

	out_rayCastRes.m_rayForwardNormal = forwardNormal;
	out_rayCastRes.m_rayMaxLength = maxDist;
	out_rayCastRes.m_rayStartPos = startPos;

	return true;
}

