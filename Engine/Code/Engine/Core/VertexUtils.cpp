#include "Engine\Core\VertexUtils.hpp"
#include "Engine\Core\Vertex_PCU.hpp"
#include "Engine\Math\MathUtils.hpp"
#include "Engine\Math\Vec2.hpp"
#include "Engine\Math\AABB2.hpp"
#include "Engine\Math\OBB2.hpp"
#include "Engine\Math\AABB3.hpp"
#include "Engine\Math\OBB3.hpp"
#include "Engine\Math\ConvexHull2.hpp"

Rgba8 g_wireColor = Rgba8( 128, 128, 128 );


void TransformVertexArrayXY3D( int numVerts, Vertex_PCU* verts, float uniformScaleXY, float rotationDegreesAboutZ, Vec2 const& translationXY )
{
	Vec2 iBasis = Vec2( CosDegrees( rotationDegreesAboutZ ) * uniformScaleXY, SinDegrees( rotationDegreesAboutZ ) * uniformScaleXY );
	Vec2 jBasis = Vec2( -iBasis.y, iBasis.x );
	for (int i = 0; i < numVerts; i++) {
		TransformPositionXY3D( verts[i].m_position, iBasis, jBasis, translationXY );
	}
}

void TransformVertexArrayXY3D( std::vector<Vertex_PCU>& verts, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY )
{
	for (Vertex_PCU& i: verts) {
		TransformPositionXY3D( i.m_position, iBasis, jBasis, translationXY );
	}
}

void TransformVertexArrayXY3D( int numVerts, Vertex_PCU* verts, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY )
{
	for (int i = 0; i < numVerts; i++) {
		TransformPositionXY3D( verts[i].m_position, iBasis, jBasis, translationXY );
	}
}

void TransformVertexArray3D( std::vector<Vertex_PCU>& verts, Vec3 const& position, EulerAngles const& rotation )
{
	Mat44 transformMatrix = Mat44::CreateTranslation3D( position );
	transformMatrix.Append( rotation.GetAsMatrix_IFwd_JLeft_KUp() );
	for (auto& vert : verts) {
		vert.m_position = transformMatrix.TransformPosition3D( vert.m_position );
	}
}

void TransformVertexArray3D( std::vector<Vertex_PCU>& verts, Mat44 const& transform )
{
	for (auto& vert : verts) {
		transform.TransformPosition3D( vert.m_position );
	}
}

AABB2 const GetVertexBounds2D( std::vector<Vertex_PCU> const& verts )
{
	if ((int)verts.size() == 0) {
		return AABB2();
	}
	Vec2 minBound = Vec2(verts[0].m_position.x, verts[0].m_position.y);
	Vec2 maxBound = minBound;
	for (auto const& vert : verts) {
		if (vert.m_position.x < minBound.x) {
			minBound.x = vert.m_position.x;
		}
		if (vert.m_position.x > maxBound.x) {
			maxBound.x = vert.m_position.x;
		}
		if (vert.m_position.y < minBound.y) {
			minBound.y = vert.m_position.y;
		}
		if (vert.m_position.y > maxBound.y) {
			maxBound.y = vert.m_position.y;
		}
	}
	return AABB2( minBound, maxBound );
}

/*void TransformPartsOfVertexArrayXY3D(std::vector<Vertex_PCU>::iterator& beginIter, std::vector<Vertex_PCU>::iterator& endIter, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY)
{
	for (std::vector<Vertex_PCU>::iterator i = beginIter; i != endIter; i++) {
		TransformPositionXY3D( i->m_position, iBasis, jBasis, translationXY );
	}
}*/

void AddVertsForCapsule2D( std::vector<Vertex_PCU>& verts, Vec2 const& boneStartPoint, Vec2 const& boneEndPoint, float radius, Rgba8 const& color )
{
	Vec2 forwardVector = boneEndPoint - boneStartPoint;
	float length = forwardVector.GetLength();
	Vec2 iBasisNormal = forwardVector / length;
	Vec2 jBasisNormal = Vec2( -iBasisNormal.y, iBasisNormal.x );
	float startRadian = forwardVector.GetOrientationRadians() - PI * 0.5f;
	constexpr int NUM_OF_VERTS = 90;
	for (int i = 0; i < NUM_OF_VERTS / 6; i++) {
		verts.emplace_back( boneEndPoint, color );
		verts.emplace_back( boneEndPoint + Vec2( CosRadians( 6 * PI / NUM_OF_VERTS * i + startRadian ) * radius, SinRadians( 6 * PI / NUM_OF_VERTS * i + startRadian ) * radius ), color );
		verts.emplace_back( boneEndPoint + Vec2( CosRadians( 6 * PI / NUM_OF_VERTS * (i + 1) + startRadian ) * radius, SinRadians( 6 * PI / NUM_OF_VERTS * (i + 1) + startRadian ) * radius ), color );
		}
	for (int i = NUM_OF_VERTS / 6; i < NUM_OF_VERTS / 3; i++) {
		verts.emplace_back( boneStartPoint, color );
		verts.emplace_back( boneStartPoint + Vec2( CosRadians( 6 * PI / NUM_OF_VERTS * i + startRadian ) * radius, SinRadians( 6 * PI / NUM_OF_VERTS * i + startRadian ) * radius ), color );
		verts.emplace_back( boneStartPoint + Vec2( CosRadians( 6 * PI / NUM_OF_VERTS * (i + 1) + startRadian ) * radius, SinRadians( 6 * PI / NUM_OF_VERTS * (i + 1) + startRadian ) * radius ), color );
	}
	verts.emplace_back( boneStartPoint - jBasisNormal * radius, color );
	verts.emplace_back( boneEndPoint - jBasisNormal * radius, color );
	verts.emplace_back( boneStartPoint + jBasisNormal * radius, color );
	verts.emplace_back( boneStartPoint + jBasisNormal * radius, color );
	verts.emplace_back( boneEndPoint - jBasisNormal * radius, color );
	verts.emplace_back( boneEndPoint + jBasisNormal * radius, color );
}

void AddVertsForDisc2D( std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, Rgba8 const& color )
{
	constexpr int NUM_OF_VERTS = 60;
	for (int i = 0; i < NUM_OF_VERTS / 3; i++) {
		verts.emplace_back( center, color );
		verts.emplace_back( center + Vec2( CosRadians( 6 * PI / NUM_OF_VERTS * i ) * radius, SinRadians( 6 * PI / NUM_OF_VERTS * i ) * radius ), color );
		verts.emplace_back( center + Vec2( CosRadians( 6 * PI / NUM_OF_VERTS * (i + 1) ) * radius, SinRadians( 6 * PI / NUM_OF_VERTS * (i + 1) ) * radius ), color );
	}
}

void AddVertsForAABB2D( std::vector<Vertex_PCU>& verts, AABB2 const& aabbBox, Rgba8 const& color, Vec2 const& uvAtMins, Vec2 const& uvAtMaxs )
{
	verts.emplace_back( aabbBox.m_mins, color, uvAtMins );
	verts.emplace_back( Vec2( aabbBox.m_maxs.x, aabbBox.m_mins.y ), color, Vec2( uvAtMaxs.x, uvAtMins.y ) );
	verts.emplace_back( Vec2( aabbBox.m_mins.x, aabbBox.m_maxs.y ), color, Vec2( uvAtMins.x, uvAtMaxs.y ) );
	verts.emplace_back( aabbBox.m_maxs, color, uvAtMaxs );
	verts.emplace_back( Vec2( aabbBox.m_mins.x, aabbBox.m_maxs.y ), color, Vec2( uvAtMins.x, uvAtMaxs.y ) );
	verts.emplace_back( Vec2( aabbBox.m_maxs.x, aabbBox.m_mins.y ), color, Vec2( uvAtMaxs.x, uvAtMins.y ) );
}

void AddVertsForAABB2D( std::vector<Vertex_PCU>& verts, AABB2 const& aabbBox, Rgba8 const& color, AABB2 const& uvs )
{
	AddVertsForAABB2D( verts, aabbBox, color, uvs.m_mins, uvs.m_maxs );
}

void AddVertsForOBB2D( std::vector<Vertex_PCU>& verts, OBB2 const& obbBox, Rgba8 const& color, Vec2 const& uvAtMins, Vec2 const& uvAtMaxs )
{
	Vec2 forwardVector = obbBox.m_iBasisNormal * obbBox.m_halfDimensions.x;
	Vec2 upwardVector = Vec2( -obbBox.m_iBasisNormal.y, obbBox.m_iBasisNormal.x ) * obbBox.m_halfDimensions.y;
	verts.emplace_back( obbBox.m_center - forwardVector - upwardVector, color, uvAtMins );
	verts.emplace_back( obbBox.m_center + forwardVector - upwardVector, color, Vec2( uvAtMaxs.x, uvAtMins.y ) );
	verts.emplace_back( obbBox.m_center - forwardVector + upwardVector, color, Vec2( uvAtMins.x, uvAtMaxs.y ) );
	verts.emplace_back( obbBox.m_center + forwardVector + upwardVector, color, uvAtMaxs );
	verts.emplace_back( obbBox.m_center - forwardVector + upwardVector, color, Vec2( uvAtMins.x, uvAtMaxs.y ) );
	verts.emplace_back( obbBox.m_center + forwardVector - upwardVector, color, Vec2( uvAtMaxs.x, uvAtMins.y ) );
}

void AddVertsForOBB2D( std::vector<Vertex_PCU>& verts, OBB2 const& obbBox, Rgba8 const& color, AABB2 const& uvs )
{
	AddVertsForOBB2D( verts, obbBox, color, uvs.m_mins, uvs.m_maxs );
}

void AddVertsForLineSegment2D( std::vector<Vertex_PCU>& verts, Vec2 const& startPoint, Vec2 const& endPoint, float thickness, Rgba8 const& color )
{
	Vec2 dForward = (endPoint - startPoint).GetNormalized();
	Vec2 forward = dForward * thickness * 0.5f;
	Vec2 left = dForward.GetRotated90Degrees() * thickness * 0.5f;
	Vec2 p1 = Vec2( (startPoint - forward + left).x, (startPoint - forward + left).y );
	Vec2 p2 = Vec2( (startPoint - forward - left).x, (startPoint - forward - left).y );
	Vec2 p3 = Vec2( (endPoint + forward + left).x, (endPoint + forward + left).y );
	Vec2 p4 = Vec2( (endPoint + forward - left).x, (endPoint + forward - left).y );
	verts.emplace_back( p1, color );
	verts.emplace_back( p2, color );
	verts.emplace_back( p3, color );
	verts.emplace_back( p2, color );
	verts.emplace_back( p4, color );
	verts.emplace_back( p3, color );

}

void AddVertsForSector2D( std::vector<Vertex_PCU>& verts, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius, Rgba8 const& color )
{
	constexpr int NUM_OF_VERTS = 72;
	float startDegrees = sectorForwardDegrees - sectorApertureDegrees * 0.5f;
	float stepDegrees = sectorApertureDegrees / NUM_OF_VERTS * 3.f;
	for (int i = 0; i < NUM_OF_VERTS / 3; i++) {
		verts.emplace_back( sectorTip, color );
		verts.emplace_back( sectorTip + Vec2::MakeFromPolarDegrees( startDegrees + stepDegrees * i, sectorRadius ), color );
		verts.emplace_back( sectorTip + Vec2::MakeFromPolarDegrees( startDegrees + stepDegrees * (i + 1), sectorRadius ), color );
	}
}

void AddVertsForSector2D( std::vector<Vertex_PCU>& verts, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius, Rgba8 const& color )
{
	AddVertsForSector2D( verts, sectorTip, sectorForwardNormal.GetOrientationDegrees(), sectorApertureDegrees, sectorRadius, color );
}

void AddVertsForArrow2D( std::vector<Vertex_PCU>& verts, Vec2 const& tailPos, Vec2 const& tipPos, float arrowSize, float lineThickness, Rgba8 const& color )
{
	AddVertsForLineSegment2D( verts, tailPos, tipPos, lineThickness, color );
	AddVertsForLineSegment2D( verts, tipPos, tipPos + Vec2::MakeFromPolarDegrees( (tipPos - tailPos).GetOrientationDegrees() + 135.f ) * arrowSize, lineThickness, color );
	AddVertsForLineSegment2D( verts, tipPos, tipPos + Vec2::MakeFromPolarDegrees( (tipPos - tailPos).GetOrientationDegrees() - 135.f ) * arrowSize, lineThickness, color );
}

void AddVertsForConvexPoly2D( std::vector<Vertex_PCU>& verts, ConvexPoly2 const& convexPoly2, Rgba8 const& color )
{
	int vertexCount = convexPoly2.GetVertexCount();
	if (vertexCount <= 2) { // cannot draw a polygon with less than 3 vertices
		return;
	}
	std::vector<Vec2> const& vertexArray = convexPoly2.GetVertexArray();
	for (int i = 1; i < vertexCount - 1; ++i) {
		verts.emplace_back( vertexArray[0], color );
		verts.emplace_back( vertexArray[i], color );
		verts.emplace_back( vertexArray[i + 1], color );
	}
}

void AddVertsForTriangle3D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec3 const& a, Vec3 const& b, Vec3 const& c, Rgba8 const& color /*= Rgba8::WHITE*/, Vec2 const& uv1, Vec2 const& uv2, Vec2 const& uv3 )
{
	unsigned int firstIndex = (unsigned int)verts.size();
	verts.emplace_back( a, color, uv1, CrossProduct3D( b - a, c - a ).GetNormalized() );
	verts.emplace_back( b, color, uv2, CrossProduct3D( c - b, a - b ).GetNormalized() );
	verts.emplace_back( c, color, uv3, CrossProduct3D( a - c, b - c ).GetNormalized() );
	indexes.push_back( firstIndex );
	indexes.push_back( firstIndex + 1 );
	indexes.push_back( firstIndex + 2 );
}

void AddVertsForQuad3D( std::vector<Vertex_PCU>& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color /*= Rgba8::WHITE*/, AABB2 const& uvs /*= AABB2::IDENTITY */ )
{
	verts.emplace_back( bottomLeft, color, uvs.m_mins );
	verts.emplace_back( bottomRight, color, Vec2( uvs.m_maxs.x, uvs.m_mins.y ) );
	verts.emplace_back( topRight, color, uvs.m_maxs );
	verts.emplace_back( topLeft, color, Vec2( uvs.m_mins.x, uvs.m_maxs.y ) );
	verts.emplace_back( bottomLeft, color, uvs.m_mins );
	verts.emplace_back( topRight, color, uvs.m_maxs );
}

void AddVertsForQuad3D( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color /*= Rgba8::WHITE*/, AABB2 const& uvs /*= AABB2::IDENTITY*/ )
{
	unsigned int firstIndex = (unsigned int)verts.size();
	verts.emplace_back( bottomLeft, color, uvs.m_mins );
	verts.emplace_back( bottomRight, color, Vec2( uvs.m_maxs.x, uvs.m_mins.y ) );
	verts.emplace_back( topRight, color, uvs.m_maxs );
	verts.emplace_back( topLeft, color, Vec2( uvs.m_mins.x, uvs.m_maxs.y ) );
	indexes.push_back( firstIndex );
	indexes.push_back( firstIndex + 1 );
	indexes.push_back( firstIndex + 2 );
	indexes.push_back( firstIndex + 3 );
	indexes.push_back( firstIndex );
	indexes.push_back( firstIndex + 2 );
}

void AddVertsForQuad3D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color /*= Rgba8::WHITE*/, AABB2 const& uvs /*= AABB2::IDENTITY */ )
{
	unsigned int firstIndex = (unsigned int)verts.size();
	verts.emplace_back( bottomLeft, color, uvs.m_mins, CrossProduct3D( bottomRight - bottomLeft, topLeft - bottomLeft ).GetNormalized() );
	verts.emplace_back( bottomRight, color, Vec2( uvs.m_maxs.x, uvs.m_mins.y ), CrossProduct3D( topRight - bottomRight, bottomLeft - bottomRight ).GetNormalized() );
	verts.emplace_back( topRight, color, uvs.m_maxs, CrossProduct3D( topLeft - topRight, bottomRight - topRight ).GetNormalized() );
	verts.emplace_back( topLeft, color, Vec2( uvs.m_mins.x, uvs.m_maxs.y ), CrossProduct3D( bottomLeft - topLeft, topRight - topLeft ).GetNormalized() );
	indexes.push_back( firstIndex );
	indexes.push_back( firstIndex + 1 );
	indexes.push_back( firstIndex + 2 );
	indexes.push_back( firstIndex + 3 );
	indexes.push_back( firstIndex );
	indexes.push_back( firstIndex + 2 );
}

void AddVertsForQuad3D( std::vector<Vertex_PCUTBN>& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color /*= Rgba8::WHITE*/, AABB2 const& uvs /*= AABB2::IDENTITY */ )
{
	verts.emplace_back( bottomLeft, color, uvs.m_mins, CrossProduct3D( bottomRight - bottomLeft, topLeft - bottomLeft ).GetNormalized() );
	verts.emplace_back( bottomRight, color, Vec2( uvs.m_maxs.x, uvs.m_mins.y ), CrossProduct3D( topRight - bottomRight, bottomLeft - bottomRight ).GetNormalized() );
	verts.emplace_back( topRight, color, uvs.m_maxs, CrossProduct3D( topLeft - topRight, bottomRight - topRight ).GetNormalized() );
	verts.emplace_back( topLeft, color, Vec2( uvs.m_mins.x, uvs.m_maxs.y ), CrossProduct3D( bottomLeft - topLeft, topRight - topLeft ).GetNormalized() );
	verts.emplace_back( bottomLeft, color, uvs.m_mins, CrossProduct3D( bottomRight - bottomLeft, topLeft - bottomLeft ).GetNormalized() );
	verts.emplace_back( topRight, color, uvs.m_maxs, CrossProduct3D( topLeft - topRight, bottomRight - topRight ).GetNormalized() );
}

void AddVertsForRoundedQuad3D( std::vector<Vertex_PCUTBN>& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color /*= Rgba8::WHITE*/, AABB2 const& uvs /*= AABB2::IDENTITY */ )
{
	Vec3 middleTop = (topRight + topLeft) * 0.5f;
	Vec3 middleBottom = (bottomRight + bottomLeft) * 0.5f;
	Vec3 normal = CrossProduct3D( bottomRight - bottomLeft, topLeft - bottomLeft ).GetNormalized();
	verts.emplace_back( bottomLeft, color, uvs.m_mins, (bottomLeft - bottomRight).GetNormalized() );
	verts.emplace_back( middleBottom, color, Vec2( (uvs.m_mins.x + uvs.m_maxs.x) * 0.5f, uvs.m_mins.y ), normal );
	verts.emplace_back( middleTop, color, Vec2( (uvs.m_mins.x + uvs.m_maxs.x) * 0.5f, uvs.m_maxs.y ), normal );
	verts.emplace_back( bottomLeft, color, uvs.m_mins, (bottomLeft - bottomRight).GetNormalized() );
	verts.emplace_back( middleTop, color, Vec2( (uvs.m_mins.x + uvs.m_maxs.x) * 0.5f, uvs.m_maxs.y ), normal );
	verts.emplace_back( topLeft, color, Vec2( uvs.m_mins.x, uvs.m_maxs.y ), (topLeft - topRight).GetNormalized() );


	verts.emplace_back( middleBottom, color, Vec2( (uvs.m_mins.x + uvs.m_maxs.x) * 0.5f, uvs.m_mins.y ), normal );
	verts.emplace_back( bottomRight, color, Vec2( uvs.m_maxs.x, uvs.m_mins.y ), (bottomRight - bottomLeft).GetNormalized() );
	verts.emplace_back( topRight, color, uvs.m_maxs, (topRight - topLeft).GetNormalized() );
	verts.emplace_back( middleBottom, color, Vec2( (uvs.m_mins.x + uvs.m_maxs.x) * 0.5f, uvs.m_mins.y ), normal );
	verts.emplace_back( topRight, color, uvs.m_maxs, (topRight - topLeft).GetNormalized() );
	verts.emplace_back( middleTop, color, Vec2( (uvs.m_mins.x + uvs.m_maxs.x) * 0.5f, uvs.m_maxs.y ), normal );

}

void AddVertsForWiredQuad3D( std::vector<Vertex_PCU>& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color /*= Rgba8::WHITE*/, float wireRadius /*= 0.01f */ )
{
	AddVertsForLineSegment3D( verts, bottomLeft, bottomRight, wireRadius, color, 8 );
	AddVertsForLineSegment3D( verts, bottomRight, topRight, wireRadius, color, 8 );
	AddVertsForLineSegment3D( verts, topLeft, bottomLeft, wireRadius, color, 8 );
	AddVertsForLineSegment3D( verts, topRight, topLeft, wireRadius, color, 8 );
}

void AddVertsForAABB3D( std::vector<Vertex_PCU>& verts, AABB3 const& bounds, Rgba8 const& color /*= Rgba8::WHITE*/, AABB2 const& uvs /*= AABB2::IDENTITY */ )
{
	Vec3 minXminYminZ = bounds.m_mins;
	Vec3 minXminYmaxZ = Vec3( bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z );
	Vec3 minXmaxYminZ = Vec3( bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z );
	Vec3 minXmaxYmaxZ = Vec3( bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z );
	Vec3 maxXminYminZ = Vec3( bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z );
	Vec3 maxXminYmaxZ = Vec3( bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z );
	Vec3 maxXmaxYminZ = Vec3( bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z );
	Vec3 maxXmaxYmaxZ = bounds.m_maxs;
	AddVertsForQuad3D( verts, minXminYmaxZ, maxXminYmaxZ, maxXmaxYmaxZ, minXmaxYmaxZ, color, uvs );
	AddVertsForQuad3D( verts, minXmaxYminZ, maxXmaxYminZ, maxXminYminZ, minXminYminZ, color, uvs );
	AddVertsForQuad3D( verts, maxXminYminZ, maxXmaxYminZ, maxXmaxYmaxZ, maxXminYmaxZ, color, uvs );
	AddVertsForQuad3D( verts, minXmaxYminZ, minXminYminZ, minXminYmaxZ, minXmaxYmaxZ, color, uvs );
	AddVertsForQuad3D( verts, maxXmaxYminZ, minXmaxYminZ, minXmaxYmaxZ, maxXmaxYmaxZ, color, uvs );
	AddVertsForQuad3D( verts, minXminYminZ, maxXminYminZ, maxXminYmaxZ, minXminYmaxZ, color, uvs );
}

void AddVertsForAABB3D( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, AABB3 const& bounds, Rgba8 const& color /*= Rgba8::WHITE*/, AABB2 const& uvs /*= AABB2::IDENTITY*/ )
{
	Vec3 minXminYminZ = bounds.m_mins;
	Vec3 minXminYmaxZ = Vec3( bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z );
	Vec3 minXmaxYminZ = Vec3( bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z );
	Vec3 minXmaxYmaxZ = Vec3( bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z );
	Vec3 maxXminYminZ = Vec3( bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z );
	Vec3 maxXminYmaxZ = Vec3( bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z );
	Vec3 maxXmaxYminZ = Vec3( bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z );
	Vec3 maxXmaxYmaxZ = bounds.m_maxs;
	AddVertsForQuad3D( verts, indexes, minXminYmaxZ, maxXminYmaxZ, maxXmaxYmaxZ, minXmaxYmaxZ, color, uvs );
	AddVertsForQuad3D( verts, indexes, minXmaxYminZ, maxXmaxYminZ, maxXminYminZ, minXminYminZ, color, uvs );
	AddVertsForQuad3D( verts, indexes, maxXminYminZ, maxXmaxYminZ, maxXmaxYmaxZ, maxXminYmaxZ, color, uvs );
	AddVertsForQuad3D( verts, indexes, minXmaxYminZ, minXminYminZ, minXminYmaxZ, minXmaxYmaxZ, color, uvs );
	AddVertsForQuad3D( verts, indexes, maxXmaxYminZ, minXmaxYminZ, minXmaxYmaxZ, maxXmaxYmaxZ, color, uvs );
	AddVertsForQuad3D( verts, indexes, minXminYminZ, maxXminYminZ, maxXminYmaxZ, minXminYmaxZ, color, uvs );
}

void AddVertsForAABB3D( std::vector<Vertex_PCUTBN>& verts, AABB3 const& bounds, Rgba8 const& color /*= Rgba8::WHITE*/, AABB2 const& uvs /*= AABB2::IDENTITY */ )
{
	Vec3 minXminYminZ = bounds.m_mins;
	Vec3 minXminYmaxZ = Vec3( bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z );
	Vec3 minXmaxYminZ = Vec3( bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z );
	Vec3 minXmaxYmaxZ = Vec3( bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z );
	Vec3 maxXminYminZ = Vec3( bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z );
	Vec3 maxXminYmaxZ = Vec3( bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z );
	Vec3 maxXmaxYminZ = Vec3( bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z );
	Vec3 maxXmaxYmaxZ = bounds.m_maxs;
	AddVertsForQuad3D( verts, minXminYmaxZ, maxXminYmaxZ, maxXmaxYmaxZ, minXmaxYmaxZ, color, uvs );
	AddVertsForQuad3D( verts, minXmaxYminZ, maxXmaxYminZ, maxXminYminZ, minXminYminZ, color, uvs );
	AddVertsForQuad3D( verts, maxXminYminZ, maxXmaxYminZ, maxXmaxYmaxZ, maxXminYmaxZ, color, uvs );
	AddVertsForQuad3D( verts, minXmaxYminZ, minXminYminZ, minXminYmaxZ, minXmaxYmaxZ, color, uvs );
	AddVertsForQuad3D( verts, maxXmaxYminZ, minXmaxYminZ, minXmaxYmaxZ, maxXmaxYmaxZ, color, uvs );
	AddVertsForQuad3D( verts, minXminYminZ, maxXminYminZ, maxXminYmaxZ, minXminYmaxZ, color, uvs );
}

void AddVertsForAABB3D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, AABB3 const& bounds, Rgba8 const& color /*= Rgba8::WHITE*/, AABB2 const& uvs /*= AABB2::IDENTITY */ )
{
	Vec3 minXminYminZ = bounds.m_mins;
	Vec3 minXminYmaxZ = Vec3( bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z );
	Vec3 minXmaxYminZ = Vec3( bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z );
	Vec3 minXmaxYmaxZ = Vec3( bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z );
	Vec3 maxXminYminZ = Vec3( bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z );
	Vec3 maxXminYmaxZ = Vec3( bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z );
	Vec3 maxXmaxYminZ = Vec3( bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z );
	Vec3 maxXmaxYmaxZ = bounds.m_maxs;
	AddVertsForQuad3D( verts, indexes, minXminYmaxZ, maxXminYmaxZ, maxXmaxYmaxZ, minXmaxYmaxZ, color, uvs );
	AddVertsForQuad3D( verts, indexes, minXmaxYminZ, maxXmaxYminZ, maxXminYminZ, minXminYminZ, color, uvs );
	AddVertsForQuad3D( verts, indexes, maxXminYminZ, maxXmaxYminZ, maxXmaxYmaxZ, maxXminYmaxZ, color, uvs );
	AddVertsForQuad3D( verts, indexes, minXmaxYminZ, minXminYminZ, minXminYmaxZ, minXmaxYmaxZ, color, uvs );
	AddVertsForQuad3D( verts, indexes, maxXmaxYminZ, minXmaxYminZ, minXmaxYmaxZ, maxXmaxYmaxZ, color, uvs );
	AddVertsForQuad3D( verts, indexes, minXminYminZ, maxXminYminZ, maxXminYmaxZ, minXminYmaxZ, color, uvs );
}

void AddVertsForWiredAABB3D( std::vector<Vertex_PCU>& verts, AABB3 const& bounds, Rgba8 const& color /*= Rgba8::WHITE */ )
{
	Vec3 minXminYminZ = bounds.m_mins;
	Vec3 minXminYmaxZ = Vec3( bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z );
	Vec3 minXmaxYminZ = Vec3( bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z );
	Vec3 minXmaxYmaxZ = Vec3( bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z );
	Vec3 maxXminYminZ = Vec3( bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z );
	Vec3 maxXminYmaxZ = Vec3( bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z );
	Vec3 maxXmaxYminZ = Vec3( bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z );
	Vec3 maxXmaxYmaxZ = bounds.m_maxs;
	float diagonalLength = (bounds.m_maxs - bounds.m_mins).GetLength();
	AddVertsForWiredQuad3D( verts, minXminYmaxZ, maxXminYmaxZ, maxXmaxYmaxZ, minXmaxYmaxZ, color, diagonalLength / 200.f );
	AddVertsForWiredQuad3D( verts, minXmaxYminZ, maxXmaxYminZ, maxXminYminZ, minXminYminZ, color, diagonalLength / 200.f );
	AddVertsForWiredQuad3D( verts, maxXminYminZ, maxXmaxYminZ, maxXmaxYmaxZ, maxXminYmaxZ, color, diagonalLength / 200.f );
	AddVertsForWiredQuad3D( verts, minXmaxYminZ, minXminYminZ, minXminYmaxZ, minXmaxYmaxZ, color, diagonalLength / 200.f );
	AddVertsForWiredQuad3D( verts, maxXmaxYminZ, minXmaxYminZ, minXmaxYmaxZ, maxXmaxYmaxZ, color, diagonalLength / 200.f );
	AddVertsForWiredQuad3D( verts, minXminYminZ, maxXminYminZ, maxXminYmaxZ, minXminYmaxZ, color, diagonalLength / 200.f );
}

void AddVertsForSphere3D( std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, Rgba8 const& color /*= Rgba8::WHITE*/, AABB2 const& uvs /*= AABB2::IDENTITY*/, int numLatitudeSlices /*= 8 */, int numLongitudeSlices )
{
	int NUM_OF_LATITUDE_SLICES = numLatitudeSlices;
	int NUM_OF_LONGITUDE_SLICES = numLongitudeSlices;

	float STEP_DEGREES_EACH_LONGITUDE = 360.f / (float)NUM_OF_LONGITUDE_SLICES;
	float STEP_DEGREES_EACH_LATITUDE = 180.f / (float)NUM_OF_LATITUDE_SLICES;
	constexpr float LATITUDE_START_DEGREES = 90.f;

	Vec2 sizeOfuv = uvs.m_maxs - uvs.m_mins;
	
	for (int k = 0; k < NUM_OF_LATITUDE_SLICES; k++) {
		float bottomLatitudeDegrees = LATITUDE_START_DEGREES - k * STEP_DEGREES_EACH_LATITUDE;
		float topLatitudeDegrees = LATITUDE_START_DEGREES - (k + 1) * STEP_DEGREES_EACH_LATITUDE;

		for (int i = 0; i < NUM_OF_LONGITUDE_SLICES; i++) {
			float leftLongitudeDegrees = i * STEP_DEGREES_EACH_LONGITUDE;
			float rightLongitudeDegrees = (i + 1) * STEP_DEGREES_EACH_LONGITUDE;

			Vec3 bottomLeft = center + Vec3::MakeFromPolarDegrees( bottomLatitudeDegrees, leftLongitudeDegrees, radius );
			Vec3 bottomRight = center + Vec3::MakeFromPolarDegrees( bottomLatitudeDegrees, rightLongitudeDegrees, radius );
			Vec3 topRight = center + Vec3::MakeFromPolarDegrees( topLatitudeDegrees, rightLongitudeDegrees, radius );
			Vec3 topLeft = center + Vec3::MakeFromPolarDegrees( topLatitudeDegrees, leftLongitudeDegrees, radius );

			AddVertsForQuad3D( verts, bottomLeft, bottomRight, topRight, topLeft, color, 
				AABB2(Vec2(uvs.m_mins.x + sizeOfuv.x * (1.f / NUM_OF_LONGITUDE_SLICES * i), uvs.m_mins.y + sizeOfuv.y * (1.f / NUM_OF_LATITUDE_SLICES * k)),
					Vec2( uvs.m_mins.x + sizeOfuv.x * (1.f / NUM_OF_LONGITUDE_SLICES * (i + 1)), uvs.m_mins.y + sizeOfuv.y * (1.f / NUM_OF_LATITUDE_SLICES * (k + 1)) ) ) );
		}
	}
}

void AddVertsForSphere3D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec3 const& center, float radius, Rgba8 const& color /*= Rgba8::WHITE*/, AABB2 const& uvs /*= AABB2::IDENTITY*/, int numLatitudeSlices /*= 8*/, int numLongitudeSlices /*= 16 */ )
{
	int NUM_OF_LATITUDE_SLICES = numLatitudeSlices;
	int NUM_OF_LONGITUDE_SLICES = numLongitudeSlices;

	float STEP_DEGREES_EACH_LONGITUDE = 360.f / (float)NUM_OF_LONGITUDE_SLICES;
	float STEP_DEGREES_EACH_LATITUDE = 180.f / (float)NUM_OF_LATITUDE_SLICES;
	constexpr float LATITUDE_START_DEGREES = 90.f;

	Vec2 sizeOfuv = uvs.m_maxs - uvs.m_mins;

	for (int k = 0; k < NUM_OF_LATITUDE_SLICES; k++) {
		float bottomLatitudeDegrees = LATITUDE_START_DEGREES - k * STEP_DEGREES_EACH_LATITUDE;
		float topLatitudeDegrees = LATITUDE_START_DEGREES - (k + 1) * STEP_DEGREES_EACH_LATITUDE;

		for (int i = 0; i < NUM_OF_LONGITUDE_SLICES; i++) {
			float leftLongitudeDegrees = i * STEP_DEGREES_EACH_LONGITUDE;
			float rightLongitudeDegrees = (i + 1) * STEP_DEGREES_EACH_LONGITUDE;

			if (k == 0) {
				Vec3 bottom = center + Vec3( 0.f, 0.f, -radius );
				Vec3 topRight = center + Vec3::MakeFromPolarDegrees( topLatitudeDegrees, rightLongitudeDegrees, radius );
				Vec3 topLeft = center + Vec3::MakeFromPolarDegrees( topLatitudeDegrees, leftLongitudeDegrees, radius );
				AddVertsForTriangle3D(verts, indexes, bottom, topRight, topLeft, color, 
					Vec2( uvs.m_mins.x + sizeOfuv.x * (1.f / NUM_OF_LONGITUDE_SLICES * i),uvs.m_mins.y),
					Vec2( uvs.m_mins.x + sizeOfuv.x * (1.f / NUM_OF_LONGITUDE_SLICES) * (i + 1), uvs.m_mins.y + sizeOfuv.y * (1.f / NUM_OF_LATITUDE_SLICES) * (k + 1) ),
					Vec2( uvs.m_mins.x + sizeOfuv.x * (1.f / NUM_OF_LONGITUDE_SLICES * i), uvs.m_mins.y + sizeOfuv.y * (1.f / NUM_OF_LATITUDE_SLICES) * (k + 1) ) );
			}
			else if (k == NUM_OF_LATITUDE_SLICES - 1) {
				Vec3 bottomLeft = center + Vec3::MakeFromPolarDegrees( bottomLatitudeDegrees, leftLongitudeDegrees, radius );
				Vec3 bottomRight = center + Vec3::MakeFromPolarDegrees( bottomLatitudeDegrees, rightLongitudeDegrees, radius );
				Vec3 top = center + Vec3( 0.f, 0.f, radius );
				AddVertsForTriangle3D( verts, indexes, bottomLeft, bottomRight, top, color, 
					Vec2( uvs.m_mins.x + sizeOfuv.x * (1.f / NUM_OF_LONGITUDE_SLICES * i), uvs.m_mins.y + sizeOfuv.y * (1.f / NUM_OF_LATITUDE_SLICES * k) ),
					Vec2( uvs.m_mins.x + sizeOfuv.x * (1.f / NUM_OF_LONGITUDE_SLICES) * (i + 1), uvs.m_mins.y + sizeOfuv.y * (1.f / NUM_OF_LATITUDE_SLICES) * k ),
					Vec2( uvs.m_mins.x + sizeOfuv.x * (1.f / NUM_OF_LONGITUDE_SLICES * i), uvs.m_mins.y + sizeOfuv.y * (1.f / NUM_OF_LATITUDE_SLICES) * (k + 1) ) );
			}
			else {
				Vec3 bottomLeft = center + Vec3::MakeFromPolarDegrees( bottomLatitudeDegrees, leftLongitudeDegrees, radius );
				Vec3 bottomRight = center + Vec3::MakeFromPolarDegrees( bottomLatitudeDegrees, rightLongitudeDegrees, radius );
				Vec3 topRight = center + Vec3::MakeFromPolarDegrees( topLatitudeDegrees, rightLongitudeDegrees, radius );
				Vec3 topLeft = center + Vec3::MakeFromPolarDegrees( topLatitudeDegrees, leftLongitudeDegrees, radius );

				AddVertsForQuad3D( verts, indexes, bottomLeft, bottomRight, topRight, topLeft, color,
					AABB2( Vec2( uvs.m_mins.x + sizeOfuv.x * (1.f / NUM_OF_LONGITUDE_SLICES * i), uvs.m_mins.y + sizeOfuv.y * (1.f / NUM_OF_LATITUDE_SLICES * k) ),
						Vec2( uvs.m_mins.x + sizeOfuv.x * (1.f / NUM_OF_LONGITUDE_SLICES) * (i + 1), uvs.m_mins.y + sizeOfuv.y * (1.f / NUM_OF_LATITUDE_SLICES) * (k + 1) ) ) );
			}
		}
	}
}

void AddVertsForWiredSphere3D( std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, Rgba8 const& color /*= Rgba8::WHITE*/, int numLatitudeSlices /*= 8 */, int numLongitudeSlices )
{
	int NUM_OF_LATITUDE_SLICES = numLatitudeSlices;
	int NUM_OF_LONGITUDE_SLICES = numLongitudeSlices;

	float STEP_DEGREES_EACH_LONGITUDE = 360.f / (float)NUM_OF_LONGITUDE_SLICES;
	float STEP_DEGREES_EACH_LATITUDE = 180.f / (float)NUM_OF_LATITUDE_SLICES;
	constexpr float LATITUDE_START_DEGREES = 90.f;

	for (int k = 0; k < NUM_OF_LATITUDE_SLICES; k++) {
		float bottomLatitudeDegrees = LATITUDE_START_DEGREES - k * STEP_DEGREES_EACH_LATITUDE;
		float topLatitudeDegrees = LATITUDE_START_DEGREES - (k + 1) * STEP_DEGREES_EACH_LATITUDE;

		for (int i = 0; i < NUM_OF_LONGITUDE_SLICES; i++) {
			float leftLongitudeDegrees = i * STEP_DEGREES_EACH_LONGITUDE;
			float rightLongitudeDegrees = (i + 1) * STEP_DEGREES_EACH_LONGITUDE;

			Vec3 bottomLeft = center + Vec3::MakeFromPolarDegrees( bottomLatitudeDegrees, leftLongitudeDegrees, radius );
			Vec3 bottomRight = center + Vec3::MakeFromPolarDegrees( bottomLatitudeDegrees, rightLongitudeDegrees, radius );
			Vec3 topRight = center + Vec3::MakeFromPolarDegrees( topLatitudeDegrees, rightLongitudeDegrees, radius );
			Vec3 topLeft = center + Vec3::MakeFromPolarDegrees( topLatitudeDegrees, leftLongitudeDegrees, radius );

			AddVertsForWiredQuad3D( verts, bottomLeft, bottomRight, topRight, topLeft, color, radius / 200.f );
		}
	}
}

void AddVertsForZCylinder3D( std::vector<Vertex_PCU>& verts, Vec2 const& center, float minZ, float maxZ, float radius, Rgba8 const& color /*= Rgba8::WHITE*/, AABB2 const& uvs /*= AABB2::IDENTITY*/, int numSlices /*= 8 */ )
{
	float degreesPerSlice = 360.f / numSlices;
	Vec3 start = Vec3( center.x, center.y, minZ );
	Vec3 end = Vec3( center.x, center.y, maxZ );
	float curDegrees = 0.f;
	Vec3 sideVector = Vec3( CosDegrees( curDegrees ), SinDegrees( curDegrees ), 0.f );
	Vec3 nextSideVector;
	for (int i = 0; i < numSlices; i++) {
		curDegrees += degreesPerSlice;
		nextSideVector = Vec3( CosDegrees( curDegrees ), SinDegrees( curDegrees ), 0.f );
		Vec3 startSidePoint = start + sideVector * radius;
		Vec3 startNextSidePoint = start + nextSideVector * radius;
		Vec3 endSidePoint = end + sideVector * radius;
		Vec3 endNextSidePoint = end + nextSideVector * radius;
		// start triangle
		verts.emplace_back( start, color, Vec2( 0.5f, 0.5f ) );
		verts.emplace_back( startNextSidePoint, color, Vec2( nextSideVector.x * 0.5f + 0.5f, nextSideVector.y * 0.5f + 0.5f ) );
		verts.emplace_back( startSidePoint, color, Vec2( sideVector.x * 0.5f + 0.5f, sideVector.y * 0.5f + 0.5f ) );
		// cylinder side quad
		AddVertsForQuad3D( verts, startSidePoint, startNextSidePoint, endNextSidePoint, endSidePoint, color, AABB2( Vec2( Interpolate( uvs.m_mins.x, uvs.m_maxs.x, (float)i / numSlices ), 0.f ), Vec2( Interpolate( uvs.m_mins.x, uvs.m_maxs.x, (float)(i + 1) / numSlices ), 1.f ) ) );
		// end triangle
		verts.emplace_back( end, color, Vec2( 0.5f, 0.5f ) );
		verts.emplace_back( endSidePoint, color, Vec2( sideVector.x * 0.5f + 0.5f, sideVector.y * 0.5f + 0.5f ) );
		verts.emplace_back( endNextSidePoint, color, Vec2( nextSideVector.x * 0.5f + 0.5f, nextSideVector.y * 0.5f + 0.5f ) );
		sideVector = nextSideVector;
	}
}

void AddVertsForCylinder3D( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color /*= Rgba8::WHITE*/, AABB2 const& uvs /*= AABB2::IDENTITY*/, int numSlices /*= 8 */ )
{
	float degreesPerSlice = 360.f / numSlices;
	Vec3 forwardNormal = (end - start).GetNormalized();
	Vec3 sideVector;
	if (forwardNormal != Vec3( 0, 0, 1 ) && forwardNormal != Vec3( 0, 0, -1 )) {
		sideVector = CrossProduct3D( Vec3( 0, 0, 1 ), forwardNormal ).GetNormalized() * radius;
	}
	else {
		sideVector = CrossProduct3D( Vec3( 0, 1, 0 ), forwardNormal ).GetNormalized() * radius;
	}
	Vec3 nextSideVector;
	for (int i = 0; i < numSlices; i++) {
		nextSideVector = RotateVectorAroundAxis3D( sideVector, forwardNormal, degreesPerSlice );
		Vec3 startSidePoint = start + sideVector;
		Vec3 startNextSidePoint = start + nextSideVector;
		Vec3 endSidePoint = end + sideVector;
		Vec3 endNextSidePoint = end + nextSideVector;
		// start triangle
		verts.emplace_back( start, color, Vec2( 0.5f, 0.5f ) );
		verts.emplace_back( startNextSidePoint, color, Vec2( 0.5f + 0.5f * CosDegrees( degreesPerSlice * (i + 1) ), 0.5f + 0.5f * SinDegrees( degreesPerSlice * (i + 1) ) ) );
		verts.emplace_back( startSidePoint, color, Vec2( 0.5f + 0.5f * CosDegrees( degreesPerSlice * i ), 0.5f + 0.5f * SinDegrees( degreesPerSlice * i ) ) );
		// cylinder side quad
		AddVertsForQuad3D( verts, startSidePoint, startNextSidePoint, endNextSidePoint, endSidePoint, color, AABB2( Vec2( Interpolate( uvs.m_mins.x, uvs.m_maxs.x, (float)i / numSlices ), 0.f ), Vec2( Interpolate( uvs.m_mins.x, uvs.m_maxs.x, (float)(i + 1) / numSlices ), 1.f ) ) );
		// end triangle
		verts.emplace_back( end, color, Vec2( 0.5f, 0.5f ) );
		verts.emplace_back( endSidePoint, color, Vec2( 0.5f + 0.5f * CosDegrees( degreesPerSlice * i ), 0.5f + 0.5f * SinDegrees( degreesPerSlice * i ) ) );
		verts.emplace_back( endNextSidePoint, color, Vec2( 0.5f + 0.5f * CosDegrees( degreesPerSlice * (i + 1) ), 0.5f + 0.5f * SinDegrees( degreesPerSlice * (i + 1) ) ) );
		sideVector = nextSideVector;
	}
}

void AddVertsForWiredCylinder3D( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color /*= Rgba8::WHITE*/, int numSlices /*= 8 */ )
{
	float degreesPerSlice = 360.f / numSlices;
	Vec3 forwardNormal = (end - start).GetNormalized();
	Vec3 sideVector;
	if (forwardNormal != Vec3( 0, 0, 1 ) && forwardNormal != Vec3( 0, 0, -1 )) {
		sideVector = CrossProduct3D( forwardNormal, Vec3( 0, 0, 1 ) ).GetNormalized() * radius;
	}
	else {
		sideVector = Vec3( 0, 1, 0 ) * radius;
	}
	Vec3 nextSideVector;
	for (int i = 0; i < numSlices; i++) {
		nextSideVector = RotateVectorAroundAxis3D( sideVector, forwardNormal, degreesPerSlice );
		Vec3 startSidePoint = start + sideVector;
		Vec3 startNextSidePoint = start + nextSideVector;
		Vec3 endSidePoint = end + sideVector;
		Vec3 endNextSidePoint = end + nextSideVector;
		// start triangle
		AddVertsForLineSegment3D( verts, start, startNextSidePoint, radius / 200.f, color, 8 );
		AddVertsForLineSegment3D( verts, startNextSidePoint, startSidePoint, radius / 200.f, color, 8 );
		AddVertsForLineSegment3D( verts, startSidePoint, start, radius / 200.f, color, 8 );
		// cylinder side quad
		AddVertsForWiredQuad3D( verts, startSidePoint, startNextSidePoint, endNextSidePoint, endSidePoint, color, radius / 200.f );
		// end triangle
		AddVertsForLineSegment3D( verts, end, endSidePoint, radius / 200.f, color, 8 );
		AddVertsForLineSegment3D( verts, endSidePoint, endNextSidePoint, radius / 200.f, color, 8 );
		AddVertsForLineSegment3D( verts, endNextSidePoint, end, radius / 200.f, color, 8 );
		sideVector = nextSideVector;
	}
}

void AddVertsForCone3D( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color /*= Rgba8::WHITE*/, AABB2 const& uvs /*= AABB2::IDENTITY*/, int numSlices /*= 8 */ )
{
	float degreesPerSlice = 360.f / numSlices;
	Vec3 forwardNormal = (end - start).GetNormalized();
	Vec3 sideVector;
	if (forwardNormal != Vec3( 0, 0, 1 ) && forwardNormal != Vec3( 0, 0, -1 )) {
		sideVector = CrossProduct3D( forwardNormal, Vec3( 0, 0, 1 ) ).GetNormalized() * radius;
	}
	else {
		sideVector = CrossProduct3D( forwardNormal, Vec3( 0, 1, 0 ) ).GetNormalized() * radius;
	}
	Vec3 nextSideVector;
	for (int i = 0; i < numSlices; i++) {
		nextSideVector = RotateVectorAroundAxis3D( sideVector, forwardNormal, degreesPerSlice );
		Vec3 startSidePoint = start + sideVector;
		Vec3 startNextSidePoint = start + nextSideVector;
		// start triangle
		verts.emplace_back( start, color, Vec2( 0, 0 ) );
		verts.emplace_back( startNextSidePoint, color, Vec2( 0, 0 ) );
		verts.emplace_back( startSidePoint, color, Vec2( 0, 0 ) );
		// cone side triangle
		verts.emplace_back( end, color, Vec2( Interpolate( uvs.m_mins.x, uvs.m_maxs.x, ((float)i + 0.5f) / numSlices ), 1.f ) );
		verts.emplace_back( startSidePoint, color, Vec2( Interpolate( uvs.m_mins.x, uvs.m_maxs.x, (float)i / numSlices ), 0.f ) );
		verts.emplace_back( startNextSidePoint, color, Vec2( Interpolate( uvs.m_mins.x, uvs.m_maxs.x, (float)(i + 1) / numSlices ), 0.f ) );
		sideVector = nextSideVector;
	}
}

void AddVertsForWiredCone3D( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color /*= Rgba8::WHITE*/, int numSlices /*= 8*/ )
{
	float degreesPerSlice = 360.f / numSlices;
	Vec3 forwardNormal = (end - start).GetNormalized();
	Vec3 sideVector;
	if (forwardNormal != Vec3( 0, 0, 1 ) && forwardNormal != Vec3( 0, 0, -1 )) {
		sideVector = CrossProduct3D( forwardNormal, Vec3( 0, 0, 1 ) ).GetNormalized() * radius;
	}
	else {
		sideVector = CrossProduct3D( forwardNormal, Vec3( 0, 1, 0 ) ).GetNormalized() * radius;
	}
	Vec3 nextSideVector;
	for (int i = 0; i < numSlices; i++) {
		nextSideVector = RotateVectorAroundAxis3D( sideVector, forwardNormal, degreesPerSlice );
		Vec3 startSidePoint = start + sideVector;
		Vec3 startNextSidePoint = start + nextSideVector;
		// start triangle
		AddVertsForLineSegment3D( verts, start, startNextSidePoint, radius / 200.f, color, 8 );
		AddVertsForLineSegment3D( verts, startNextSidePoint, startSidePoint, radius / 200.f, color, 8 );
		AddVertsForLineSegment3D( verts, startSidePoint, start, radius / 200.f, color, 8 );
		// cone side triangle
		AddVertsForLineSegment3D( verts, end, startSidePoint, radius / 200.f, color, 8 );
		AddVertsForLineSegment3D( verts, startSidePoint, startNextSidePoint, radius / 200.f, color, 8 );
		AddVertsForLineSegment3D( verts, startNextSidePoint, end, radius / 200.f, color, 8 );
		sideVector = nextSideVector;
	}
}

void AddVertsForArrow3D( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, float arrowSize, Rgba8 const& color /*= Rgba8::WHITE*/, int numSlices /*= 8 */ )
{
	Vec3 forwardVector = end - start;
	float length = forwardVector.GetLength();
	Vec3 forwardNormal = forwardVector / length;
	Vec3 ArrowHeadStartPos = start + forwardNormal * (length - arrowSize);
	AddVertsForCylinder3D( verts, start, ArrowHeadStartPos, radius, color, AABB2::IDENTITY, numSlices );
	AddVertsForCone3D( verts, ArrowHeadStartPos, end, arrowSize, color, AABB2::IDENTITY, numSlices );
}

void AddVertsForLineSegment3D( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float thickness, Rgba8 const& color, int numSlices /*= 8 */ )
{
	AddVertsForCylinder3D( verts, start, end, thickness, color, AABB2::IDENTITY, numSlices );
}

void AddVertsForOBB3D( std::vector<Vertex_PCU>& verts, OBB3 const& obb3, Rgba8 const& color, AABB2 const& uvs )
{
	Vec3 minXminYminZ = obb3.m_center - obb3.m_iBasis * obb3.m_halfDimensions.x -
		obb3.m_jBasis * obb3.m_halfDimensions.y - obb3.m_kBasis * obb3.m_halfDimensions.z;
	Vec3 minXminYmaxZ = obb3.m_center - obb3.m_iBasis * obb3.m_halfDimensions.x -
		obb3.m_jBasis * obb3.m_halfDimensions.y + obb3.m_kBasis * obb3.m_halfDimensions.z;
	Vec3 minXmaxYminZ = obb3.m_center - obb3.m_iBasis * obb3.m_halfDimensions.x +
		obb3.m_jBasis * obb3.m_halfDimensions.y - obb3.m_kBasis * obb3.m_halfDimensions.z;
	Vec3 minXmaxYmaxZ = obb3.m_center - obb3.m_iBasis * obb3.m_halfDimensions.x +
		obb3.m_jBasis * obb3.m_halfDimensions.y + obb3.m_kBasis * obb3.m_halfDimensions.z;
	Vec3 maxXminYminZ = obb3.m_center + obb3.m_iBasis * obb3.m_halfDimensions.x -
		obb3.m_jBasis * obb3.m_halfDimensions.y - obb3.m_kBasis * obb3.m_halfDimensions.z;
	Vec3 maxXminYmaxZ = obb3.m_center + obb3.m_iBasis * obb3.m_halfDimensions.x -
		obb3.m_jBasis * obb3.m_halfDimensions.y + obb3.m_kBasis * obb3.m_halfDimensions.z;
	Vec3 maxXmaxYminZ = obb3.m_center + obb3.m_iBasis * obb3.m_halfDimensions.x +
		obb3.m_jBasis * obb3.m_halfDimensions.y - obb3.m_kBasis * obb3.m_halfDimensions.z;
	Vec3 maxXmaxYmaxZ = obb3.m_center + obb3.m_iBasis * obb3.m_halfDimensions.x +
		obb3.m_jBasis * obb3.m_halfDimensions.y + obb3.m_kBasis * obb3.m_halfDimensions.z;
	AddVertsForQuad3D( verts, minXminYmaxZ, maxXminYmaxZ, maxXmaxYmaxZ, minXmaxYmaxZ, color, uvs );
	AddVertsForQuad3D( verts, minXmaxYminZ, maxXmaxYminZ, maxXminYminZ, minXminYminZ, color, uvs );
	AddVertsForQuad3D( verts, maxXminYminZ, maxXmaxYminZ, maxXmaxYmaxZ, maxXminYmaxZ, color, uvs );
	AddVertsForQuad3D( verts, minXmaxYminZ, minXminYminZ, minXminYmaxZ, minXmaxYmaxZ, color, uvs );
	AddVertsForQuad3D( verts, maxXmaxYminZ, minXmaxYminZ, minXmaxYmaxZ, maxXmaxYmaxZ, color, uvs );
	AddVertsForQuad3D( verts, minXminYminZ, maxXminYminZ, maxXminYmaxZ, minXminYmaxZ, color, uvs );
}

void AddVertsForWiredOBB3D( std::vector<Vertex_PCU>& verts, OBB3 const& obb3, Rgba8 const& color )
{
	Vec3 minXminYminZ = obb3.m_center - obb3.m_iBasis * obb3.m_halfDimensions.x -
		obb3.m_jBasis * obb3.m_halfDimensions.y - obb3.m_kBasis * obb3.m_halfDimensions.z;
	Vec3 minXminYmaxZ = obb3.m_center - obb3.m_iBasis * obb3.m_halfDimensions.x -
		obb3.m_jBasis * obb3.m_halfDimensions.y + obb3.m_kBasis * obb3.m_halfDimensions.z;
	Vec3 minXmaxYminZ = obb3.m_center - obb3.m_iBasis * obb3.m_halfDimensions.x +
		obb3.m_jBasis * obb3.m_halfDimensions.y - obb3.m_kBasis * obb3.m_halfDimensions.z;
	Vec3 minXmaxYmaxZ = obb3.m_center - obb3.m_iBasis * obb3.m_halfDimensions.x +
		obb3.m_jBasis * obb3.m_halfDimensions.y + obb3.m_kBasis * obb3.m_halfDimensions.z;
	Vec3 maxXminYminZ = obb3.m_center + obb3.m_iBasis * obb3.m_halfDimensions.x -
		obb3.m_jBasis * obb3.m_halfDimensions.y - obb3.m_kBasis * obb3.m_halfDimensions.z;
	Vec3 maxXminYmaxZ = obb3.m_center + obb3.m_iBasis * obb3.m_halfDimensions.x -
		obb3.m_jBasis * obb3.m_halfDimensions.y + obb3.m_kBasis * obb3.m_halfDimensions.z;
	Vec3 maxXmaxYminZ = obb3.m_center + obb3.m_iBasis * obb3.m_halfDimensions.x +
		obb3.m_jBasis * obb3.m_halfDimensions.y - obb3.m_kBasis * obb3.m_halfDimensions.z;
	Vec3 maxXmaxYmaxZ = obb3.m_center + obb3.m_iBasis * obb3.m_halfDimensions.x +
		obb3.m_jBasis * obb3.m_halfDimensions.y + obb3.m_kBasis * obb3.m_halfDimensions.z;
	float diagonalLength = 2.f * obb3.m_halfDimensions.GetLength();
	AddVertsForWiredQuad3D( verts, minXminYmaxZ, maxXminYmaxZ, maxXmaxYmaxZ, minXmaxYmaxZ, color, diagonalLength / 200.f );
	AddVertsForWiredQuad3D( verts, minXmaxYminZ, maxXmaxYminZ, maxXminYminZ, minXminYminZ, color, diagonalLength / 200.f );
	AddVertsForWiredQuad3D( verts, maxXminYminZ, maxXmaxYminZ, maxXmaxYmaxZ, maxXminYmaxZ, color, diagonalLength / 200.f );
	AddVertsForWiredQuad3D( verts, minXmaxYminZ, minXminYminZ, minXminYmaxZ, minXmaxYmaxZ, color, diagonalLength / 200.f );
	AddVertsForWiredQuad3D( verts, maxXmaxYminZ, minXmaxYminZ, minXmaxYmaxZ, maxXmaxYmaxZ, color, diagonalLength / 200.f );
	AddVertsForWiredQuad3D( verts, minXminYminZ, maxXminYminZ, maxXminYmaxZ, minXminYmaxZ, color, diagonalLength / 200.f );
}

void CalculateTangentSpaceBasisVectors( std::vector<Vertex_PCUTBN>& vertexes, std::vector<unsigned int>& indexes, bool computeNormals, bool computeTangents )
{
	int numOfTriangles = (int)(indexes.size() / 3);
	for (int i = 0; i < numOfTriangles; i++) {
		Vec3 ab = vertexes[indexes[i * 3 + 1]].m_position - vertexes[indexes[i * 3]].m_position;
		Vec3 ac = vertexes[indexes[i * 3 + 2]].m_position - vertexes[indexes[i * 3]].m_position;
		Vec3 bc = vertexes[indexes[i * 3 + 2]].m_position - vertexes[indexes[i * 3 + 1]].m_position;

		if (computeNormals) {
			vertexes[indexes[i * 3]].m_normal = CrossProduct3D( ab, ac ).GetNormalized();
			vertexes[indexes[i * 3 + 1]].m_normal = CrossProduct3D( bc, -ab ).GetNormalized();
			vertexes[indexes[i * 3 + 2]].m_normal = CrossProduct3D( -ac, -bc ).GetNormalized();
		}

		if (computeTangents) {
			Vec3 uv_ab = vertexes[indexes[i * 3 + 1]].m_uvTexCoords - vertexes[indexes[i * 3]].m_uvTexCoords;
			Vec3 uv_ac = vertexes[indexes[i * 3 + 2]].m_uvTexCoords - vertexes[indexes[i * 3]].m_uvTexCoords;
			Vec3 uv_bc = vertexes[indexes[i * 3 + 2]].m_uvTexCoords - vertexes[indexes[i * 3 + 1]].m_uvTexCoords;

			// for a
			float multiplier = 1.f / (uv_ab.x * uv_ac.y - uv_ab.y * uv_ac.x);
			vertexes[indexes[i * 3]].m_tangent.x = multiplier * (uv_ac.y * ab.x - uv_ab.y * ac.x);
			vertexes[indexes[i * 3]].m_tangent.y = multiplier * (uv_ac.y * ab.y - uv_ab.y * ac.y);
			vertexes[indexes[i * 3]].m_tangent.z = multiplier * (uv_ac.y * ab.z - uv_ab.y * ac.z);
			vertexes[indexes[i * 3]].m_tangent = vertexes[indexes[i * 3]].m_tangent
				- (DotProduct3D( vertexes[indexes[i * 3]].m_tangent, vertexes[indexes[i * 3]].m_normal )) * vertexes[indexes[i * 3]].m_normal;
			vertexes[indexes[i * 3]].m_tangent.Normalize();
			vertexes[indexes[i * 3]].m_bitangent = CrossProduct3D( vertexes[indexes[i * 3]].m_normal, vertexes[indexes[i * 3]].m_tangent );
			// for b
			multiplier = 1.f / (-uv_ab.x * uv_bc.y + uv_ab.y * uv_bc.x);
			vertexes[indexes[i * 3 + 1]].m_tangent.x = multiplier * (uv_bc.y * -ab.x + uv_ab.y * bc.x);
			vertexes[indexes[i * 3 + 1]].m_tangent.y = multiplier * (uv_bc.y * -ab.y + uv_ab.y * bc.y);
			vertexes[indexes[i * 3 + 1]].m_tangent.z = multiplier * (uv_bc.y * -ab.z + uv_ab.y * bc.z);
			vertexes[indexes[i * 3 + 1]].m_tangent = vertexes[indexes[i * 3 + 1]].m_tangent
				- (DotProduct3D( vertexes[indexes[i * 3 + 1]].m_tangent, vertexes[indexes[i * 3 + 1]].m_normal )) * vertexes[indexes[i * 3 + 1]].m_normal;
			vertexes[indexes[i * 3 + 1]].m_tangent.Normalize();
			vertexes[indexes[i * 3 + 1]].m_bitangent = CrossProduct3D( vertexes[indexes[i * 3 + 1]].m_normal, vertexes[indexes[i * 3 + 1]].m_tangent );
			// for c
			multiplier = 1.f / (uv_ac.x * uv_bc.y - uv_ac.y * uv_bc.x);
			vertexes[indexes[i * 3 + 2]].m_tangent.x = multiplier * (uv_bc.y * ac.x - uv_ac.y * bc.x);
			vertexes[indexes[i * 3 + 2]].m_tangent.y = multiplier * (uv_bc.y * ac.y - uv_ac.y * bc.y);
			vertexes[indexes[i * 3 + 2]].m_tangent.z = multiplier * (uv_bc.y * ac.z - uv_ac.y * bc.z);
			vertexes[indexes[i * 3 + 2]].m_tangent = vertexes[indexes[i * 3 + 2]].m_tangent
				- (DotProduct3D( vertexes[indexes[i * 3 + 2]].m_tangent, vertexes[indexes[i * 3 + 2]].m_normal )) * vertexes[indexes[i * 3 + 2]].m_normal;
			vertexes[indexes[i * 3 + 2]].m_tangent.Normalize();
			vertexes[indexes[i * 3 + 2]].m_bitangent = CrossProduct3D( vertexes[indexes[i * 3 + 2]].m_normal, vertexes[indexes[i * 3 + 2]].m_tangent );

		}
	}
}

