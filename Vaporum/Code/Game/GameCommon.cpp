#include "Game/GameCommon.hpp"

void AddVertsForHexagon( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec2 const& center, float innerRadius, float thickness, Rgba8 const& color )
{
	int startIndex = (int)verts.size();
	innerRadius += thickness * 0.5f;
	float halfEdge = 0.57735026919f * innerRadius;
	float outerRadius = halfEdge * 2.f;
	Vec3 p1 = Vec2( center.x - halfEdge, center.y + innerRadius );
	Vec3 p2 = Vec2( center.x + halfEdge, center.y + innerRadius );
	Vec3 p3 = Vec2( center.x + outerRadius, center.y );
	Vec3 p4 = Vec2( center.x + halfEdge, center.y - innerRadius );
	Vec3 p5 = Vec2( center.x - halfEdge, center.y - innerRadius );
	Vec3 p6 = Vec2( center.x - outerRadius, center.y );

	innerRadius -= thickness;
	halfEdge = 0.57735026919f * innerRadius;
	outerRadius = halfEdge * 2.f;
	Vec3 p7 = Vec2( center.x - halfEdge, center.y + innerRadius );
	Vec3 p8 = Vec2( center.x + halfEdge, center.y + innerRadius );
	Vec3 p9 = Vec2( center.x + outerRadius, center.y );
	Vec3 p10 = Vec2( center.x + halfEdge, center.y - innerRadius );
	Vec3 p11 = Vec2( center.x - halfEdge, center.y - innerRadius );
	Vec3 p12 = Vec2( center.x - outerRadius, center.y );
	verts.emplace_back( p1, color ); verts.emplace_back( p2, color ); verts.emplace_back( p3, color );
	verts.emplace_back( p4, color ); verts.emplace_back( p5, color ); verts.emplace_back( p6, color );
	verts.emplace_back( p7, color ); verts.emplace_back( p8, color ); verts.emplace_back( p9, color );
	verts.emplace_back( p10, color ); verts.emplace_back( p11, color ); verts.emplace_back( p12, color );

	indexes.push_back( startIndex ); indexes.push_back( startIndex + 6 ); indexes.push_back( startIndex + 7 );
	indexes.push_back( startIndex ); indexes.push_back( startIndex + 7 ); indexes.push_back( startIndex + 1 );

	indexes.push_back( startIndex + 1 ); indexes.push_back( startIndex + 7 ); indexes.push_back( startIndex + 8 );
	indexes.push_back( startIndex + 1 ); indexes.push_back( startIndex + 8 ); indexes.push_back( startIndex + 2 );

	indexes.push_back( startIndex + 2 ); indexes.push_back( startIndex + 8 ); indexes.push_back( startIndex + 9 );
	indexes.push_back( startIndex + 2 ); indexes.push_back( startIndex + 9 ); indexes.push_back( startIndex + 3 );

	indexes.push_back( startIndex + 3 ); indexes.push_back( startIndex + 9 ); indexes.push_back( startIndex + 10 );
	indexes.push_back( startIndex + 3 ); indexes.push_back( startIndex + 10 ); indexes.push_back( startIndex + 4 );

	indexes.push_back( startIndex + 4 ); indexes.push_back( startIndex + 10 ); indexes.push_back( startIndex + 11 );
	indexes.push_back( startIndex + 4 ); indexes.push_back( startIndex + 11 ); indexes.push_back( startIndex + 5 );

	indexes.push_back( startIndex + 5 ); indexes.push_back( startIndex + 11 ); indexes.push_back( startIndex + 6 );
	indexes.push_back( startIndex + 5 ); indexes.push_back( startIndex + 6 ); indexes.push_back( startIndex );
	
}

void AddVertsForSolidHexagon( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec2 const& center, float innerRadius, float thickness, Rgba8 const& color /*= Rgba8::WHITE */ )
{
	int startIndex = (int)verts.size();
	innerRadius -= 0.5f * thickness;
	float halfEdge = 0.57735026919f * innerRadius;
	float outerRadius = halfEdge * 2.f;
	Vec3 p7 = Vec2( center.x - halfEdge, center.y + innerRadius );
	Vec3 p8 = Vec2( center.x + halfEdge, center.y + innerRadius );
	Vec3 p9 = Vec2( center.x + outerRadius, center.y );
	Vec3 p10 = Vec2( center.x + halfEdge, center.y - innerRadius );
	Vec3 p11 = Vec2( center.x - halfEdge, center.y - innerRadius );
	Vec3 p12 = Vec2( center.x - outerRadius, center.y );
	verts.emplace_back( center, color );
	verts.emplace_back( p7, color ); verts.emplace_back( p8, color ); verts.emplace_back( p9, color );
	verts.emplace_back( p10, color ); verts.emplace_back( p11, color ); verts.emplace_back( p12, color );

	indexes.push_back( startIndex ); indexes.push_back( startIndex + 2 ); indexes.push_back( startIndex + 1 );
	indexes.push_back( startIndex ); indexes.push_back( startIndex + 3 ); indexes.push_back( startIndex + 2 );
	indexes.push_back( startIndex ); indexes.push_back( startIndex + 4 ); indexes.push_back( startIndex + 3 );
	indexes.push_back( startIndex ); indexes.push_back( startIndex + 5 ); indexes.push_back( startIndex + 4 );
	indexes.push_back( startIndex ); indexes.push_back( startIndex + 6 ); indexes.push_back( startIndex + 5 );
	indexes.push_back( startIndex ); indexes.push_back( startIndex + 1 ); indexes.push_back( startIndex + 6 );
}

void DebugDrawRing(Vec2 const& center, float radius, float thickness, Rgba8 const& color ) {
	constexpr int NUM_SIDES = 16;
	constexpr int NUM_TRIANGLES = NUM_SIDES * 2;
	constexpr int NUM_VERTS = 3 * NUM_TRIANGLES;
	constexpr float DEGREES_PER_SIDE = 360.f / (float)NUM_SIDES;
	constexpr float RADIANS_PER_SIDE = DEGREES_PER_SIDE * PI / 180.f;
	Vertex_PCU ringVertsArray[NUM_VERTS];
	for (int i = 0; i < NUM_SIDES; i++) {
		ringVertsArray[6 * i] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * i ) * radius, SinRadians( RADIANS_PER_SIDE * i ) * radius, 0 ), color, Vec2( 0, 0 ) );
		ringVertsArray[6 * i + 1] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, 0 ), color, Vec2( 0, 0 ) );
		ringVertsArray[6 * i + 2] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), 0 ), color, Vec2( 0, 0 ) );
		ringVertsArray[6 * i + 3] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * radius, 0 ), color, Vec2( 0, 0 ) );
		ringVertsArray[6 * i + 4] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * i) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * i ) * (radius + thickness), 0 ), color, Vec2( 0, 0 ) );
		ringVertsArray[6 * i + 5] = Vertex_PCU( Vec3( CosRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * (radius + thickness), SinRadians( RADIANS_PER_SIDE * ((i + 1) % NUM_SIDES) ) * (radius + thickness), 0 ), color, Vec2( 0, 0 ) );
	}
	for (int i = 0; i < NUM_VERTS; i++) {
		ringVertsArray[i].m_position += Vec3( center.x, center.y, 0 );
	}
	g_theRenderer->DrawVertexArray( NUM_VERTS, ringVertsArray );
}

void DebugDrawLine(Vec2 const& startPos, float orientation, float length, float thickness, Rgba8 const& color) {
	Vertex_PCU lineVertsArray[6];
	Vec3 p1 = Vec3( -thickness * 0.5f, -thickness * 0.5f, 0 );
	Vec3 p2 = Vec3( -thickness * 0.5f, thickness * 0.5f, 0 );
	Vec3 p3 = Vec3( length + thickness * 0.5f, -thickness * 0.5f, 0 );
	Vec3 p4 = Vec3( length + thickness * 0.5f, thickness * 0.5f, 0 );
	lineVertsArray[0] = Vertex_PCU( p1, color, Vec2( 0, 0 ) );
	lineVertsArray[1] = Vertex_PCU( p2, color, Vec2( 0, 0 ) );
	lineVertsArray[2] = Vertex_PCU( p3, color, Vec2( 0, 0 ) );
	lineVertsArray[3] = Vertex_PCU( p2, color, Vec2( 0, 0 ) );
	lineVertsArray[4] = Vertex_PCU( p3, color, Vec2( 0, 0 ) );
	lineVertsArray[5] = Vertex_PCU( p4, color, Vec2( 0, 0 ) );

	TransformVertexArrayXY3D( 6, lineVertsArray, 1.f, orientation, startPos );
	g_theRenderer->DrawVertexArray( 6, lineVertsArray );
}

void DebugDrawLine( Vec2 const& startPos, Vec2 const& endPos, float thickness, Rgba8 const& color )
{
	Vec2 dForward = (endPos - startPos).GetNormalized();
	Vec2 forward = dForward * thickness * 0.5f;
	Vec2 left = dForward.GetRotated90Degrees() * thickness * 0.5f;
	Vertex_PCU lineVertsArray[6];
	Vec3 p1 = Vec3( (startPos - forward + left).x, (startPos - forward + left).y, 0 );
	Vec3 p2 = Vec3( (startPos - forward - left).x, (startPos - forward - left).y, 0 );
	Vec3 p3 = Vec3( (endPos + forward + left).x, (endPos + forward + left).y, 0 );
	Vec3 p4 = Vec3( (endPos + forward - left).x, (endPos + forward - left).y, 0 );
	lineVertsArray[0] = Vertex_PCU( p1, color, Vec2( 0, 0 ) );
	lineVertsArray[1] = Vertex_PCU( p2, color, Vec2( 0, 0 ) );
	lineVertsArray[2] = Vertex_PCU( p3, color, Vec2( 0, 0 ) );
	lineVertsArray[3] = Vertex_PCU( p2, color, Vec2( 0, 0 ) );
	lineVertsArray[4] = Vertex_PCU( p3, color, Vec2( 0, 0 ) );
	lineVertsArray[5] = Vertex_PCU( p4, color, Vec2( 0, 0 ) );

	g_theRenderer->DrawVertexArray( 6, lineVertsArray );
}

void AddVertsForUIPanelBlack( std::vector<Vertex_PCU>& verts, AABB2 const& bounds, float edgeWidth )
{
	AddVertsForAABB2D( verts, bounds, Rgba8::WHITE );
	AddVertsForAABB2D( verts, AABB2( bounds.m_mins + Vec2( edgeWidth, edgeWidth ), bounds.m_maxs - Vec2( edgeWidth, edgeWidth ) ), Rgba8::BLACK );
}

void AddVertsForUIPanelWhite( std::vector<Vertex_PCU>& verts, AABB2 const& bounds, float edgeWidth )
{
	AddVertsForAABB2D( verts, bounds, Rgba8::BLACK );
	AddVertsForAABB2D( verts, AABB2( bounds.m_mins + Vec2( edgeWidth, edgeWidth ), bounds.m_maxs - Vec2( edgeWidth, edgeWidth ) ), Rgba8::WHITE );
}

